use crate::models::{Account, Message};
use rusqlite::{params, Connection, Result};
use std::sync::Mutex;

pub struct Storage {
    conn: Mutex<Connection>,
}

impl Storage {
    pub fn new(path: &str) -> Result<Self> {
        let conn = Connection::open(path)?;
        conn.execute_batch(
            "
            CREATE TABLE IF NOT EXISTS accounts (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                email TEXT NOT NULL,
                imap_host TEXT NOT NULL,
                imap_port INTEGER NOT NULL,
                username TEXT NOT NULL,
                password TEXT NOT NULL
            );

            CREATE TABLE IF NOT EXISTS messages (
                id TEXT PRIMARY KEY,
                account_id TEXT NOT NULL,
                remote_uid INTEGER NOT NULL,
                sender_name TEXT NOT NULL,
                sender_email TEXT NOT NULL,
                subject TEXT NOT NULL,
                snippet TEXT NOT NULL,
                body TEXT NOT NULL,
                date TEXT NOT NULL,
                folder TEXT NOT NULL,
                is_unread INTEGER NOT NULL,
                is_starred INTEGER NOT NULL,
                summary TEXT NOT NULL,
                UNIQUE(account_id, folder, remote_uid)
            );

            CREATE INDEX IF NOT EXISTS idx_messages_folder ON messages(folder);
            ",
        )?;

        Ok(Self {
            conn: Mutex::new(conn),
        })
    }

    pub fn save_account(&self, acc: &Account) -> Result<()> {
        let conn = self.conn.lock().unwrap();
        conn.execute(
            "INSERT OR REPLACE INTO accounts (id, name, email, imap_host, imap_port, username, password)
             VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)",
            params![acc.id, acc.name, acc.email, acc.imap_host, acc.imap_port, acc.username, acc.password],
        )?;
        Ok(())
    }

    pub fn get_accounts(&self) -> Result<Vec<Account>> {
        let conn = self.conn.lock().unwrap();
        let mut stmt = conn.prepare("SELECT id, name, email, imap_host, imap_port, username, password FROM accounts")?;
        let rows = stmt.query_map([], |row| {
            Ok(Account {
                id: row.get(0)?,
                name: row.get(1)?,
                email: row.get(2)?,
                imap_host: row.get(3)?,
                imap_port: row.get(4)?,
                username: row.get(5)?,
                password: row.get(6)?,
            })
        })?;

        let mut list = Vec::new();
        for r in rows {
            list.push(r?);
        }
        Ok(list)
    }

    pub fn save_message(&self, msg: &Message) -> Result<()> {
        let conn = self.conn.lock().unwrap();
        conn.execute(
            "INSERT OR REPLACE INTO messages (
                id, account_id, remote_uid, sender_name, sender_email, subject, snippet, body, date, folder, is_unread, is_starred, summary
             ) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13)",
            params![
                msg.id,
                msg.account_id,
                msg.remote_uid,
                msg.sender_name,
                msg.sender_email,
                msg.subject,
                msg.snippet,
                msg.body,
                msg.date,
                msg.folder,
                if msg.is_unread { 1 } else { 0 },
                if msg.is_starred { 1 } else { 0 },
                msg.summary,
            ],
        )?;
        Ok(())
    }

    pub fn get_messages(&self, folder: &str) -> Result<Vec<Message>> {
        let conn = self.conn.lock().unwrap();
        let query = if folder == "all" {
            "SELECT id, account_id, remote_uid, sender_name, sender_email, subject, snippet, body, date, folder, is_unread, is_starred, summary FROM messages ORDER BY id DESC"
        } else if folder == "starred" {
            "SELECT id, account_id, remote_uid, sender_name, sender_email, subject, snippet, body, date, folder, is_unread, is_starred, summary FROM messages WHERE is_starred = 1 ORDER BY id DESC"
        } else {
            "SELECT id, account_id, remote_uid, sender_name, sender_email, subject, snippet, body, date, folder, is_unread, is_starred, summary FROM messages WHERE folder = ?1 ORDER BY id DESC"
        };

        let mut list = Vec::new();
        if folder == "all" || folder == "starred" {
            let mut stmt = conn.prepare(query)?;
            let rows = stmt.query_map([], |row| Self::row_to_message(row))?;
            for r in rows {
                list.push(r?);
            }
        } else {
            let mut stmt = conn.prepare(query)?;
            let rows = stmt.query_map([folder], |row| Self::row_to_message(row))?;
            for r in rows {
                list.push(r?);
            }
        }
        Ok(list)
    }

    fn row_to_message(row: &rusqlite::Row) -> rusqlite::Result<Message> {
        let is_unread: i32 = row.get(10)?;
        let is_starred: i32 = row.get(11)?;
        Ok(Message {
            id: row.get(0)?,
            account_id: row.get(1)?,
            remote_uid: row.get(2)?,
            sender_name: row.get(3)?,
            sender_email: row.get(4)?,
            subject: row.get(5)?,
            snippet: row.get(6)?,
            body: row.get(7)?,
            date: row.get(8)?,
            folder: row.get(9)?,
            is_unread: is_unread != 0,
            is_starred: is_starred != 0,
            summary: row.get(12)?,
        })
    }
}
