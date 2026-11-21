use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Account {
    pub id: String,
    pub name: String,
    pub email: String,
    pub imap_host: String,
    pub imap_port: u16,
    pub username: String,
    pub password: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Message {
    pub id: String,
    pub account_id: String,
    pub remote_uid: u32,
    pub sender_name: String,
    pub sender_email: String,
    pub subject: String,
    pub snippet: String,
    pub body: String,
    pub date: String,
    pub folder: String,
    pub is_unread: bool,
    pub is_starred: bool,
    pub summary: String,
}
