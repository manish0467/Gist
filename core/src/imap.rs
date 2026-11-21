use crate::models::{Account, Message};
use crate::storage::Storage;
use mail_parser::MessageParser;
use native_tls::TlsConnector;
use std::sync::Arc;

pub fn sync_inbox<F>(account: &Account, storage: &Arc<Storage>, notify: F) -> Result<usize, String>
where
    F: Fn(&str) + Send + Sync,
{
    let tls = TlsConnector::new().map_err(|e| format!("TLS init error: {}", e))?;
    let client = imap::connect(
        (&account.imap_host[..], account.imap_port),
        &account.imap_host,
        &tls,
    )
    .map_err(|e| format!("IMAP connection error: {}", e))?;

    let mut session = client
        .login(&account.username, &account.password)
        .map_err(|e| format!("IMAP login error: {:?}", e))?;

    session
        .select("INBOX")
        .map_err(|e| format!("Failed to select INBOX: {}", e))?;

    let search_results = session
        .search("ALL")
        .map_err(|e| format!("Search error: {}", e))?;

    let mut uids: Vec<u32> = search_results.into_iter().collect();
    uids.sort();
    let to_fetch: Vec<u32> = uids.into_iter().rev().take(15).collect();

    let parser = MessageParser::default();
    let mut synced_count = 0;

    for seq in to_fetch {
        if let Ok(messages) = session.fetch(seq.to_string(), "(UID ENVELOPE RFC822)") {
            for m in messages.iter() {
                let uid = m.uid.unwrap_or(seq);
                if let Some(body) = m.body() {
                    if let Some(parsed) = parser.parse(body) {
                        let sender_name = parsed
                            .from()
                            .and_then(|f| f.first())
                            .and_then(|addr| addr.name())
                            .unwrap_or("Unknown")
                            .to_string();
                        let sender_email = parsed
                            .from()
                            .and_then(|f| f.first())
                            .and_then(|addr| addr.address())
                            .unwrap_or("")
                            .to_string();
                        let subject = parsed.subject().unwrap_or("(No Subject)").to_string();
                        let text_body = parsed
                            .body_text(0)
                            .map(|b| b.to_string())
                            .unwrap_or_else(|| subject.clone());
                        let snippet = if text_body.len() > 120 {
                            format!("{}...", &text_body[..117])
                        } else {
                            text_body.clone()
                        };

                        let date_str = parsed
                            .date()
                            .map(|d| format!("{}-{:02}-{:02}", d.year, d.month, d.day))
                            .unwrap_or_else(|| "Recent".to_string());

                        let msg = Message {
                            id: format!("{}-{}", account.id, uid),
                            account_id: account.id.clone(),
                            remote_uid: uid,
                            sender_name,
                            sender_email,
                            subject,
                            snippet,
                            body: text_body,
                            date: date_str,
                            folder: "inbox".to_string(),
                            is_unread: true,
                            is_starred: false,
                            summary: String::new(),
                        };

                        let _ = storage.save_message(&msg);
                        synced_count += 1;
                    }
                }
            }
        }
    }

    let _ = session.logout();
    notify(&format!(r#"{{"event":"sync_finished","account":"{}","synced":{}}}"#, account.email, synced_count));
    Ok(synced_count)
}
