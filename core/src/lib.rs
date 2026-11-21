pub mod imap;
pub mod models;
pub mod storage;

use models::Account;
use storage::Storage;
use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::sync::Arc;
use std::thread;

pub type EventCallback = extern "C" fn(*const c_char);

pub struct GistEngine {
    storage: Arc<Storage>,
    event_cb: Arc<std::sync::Mutex<Option<EventCallback>>>,
}

#[no_mangle]
pub extern "C" fn gist_core_new(db_path: *const c_char) -> *mut GistEngine {
    if db_path.is_null() {
        return std::ptr::null_mut();
    }

    let path_str = match unsafe { CStr::from_ptr(db_path) }.to_str() {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };

    match Storage::new(path_str) {
        Ok(storage) => {
            let engine = Box::new(GistEngine {
                storage: Arc::new(storage),
                event_cb: Arc::new(std::sync::Mutex::new(None)),
            });
            Box::into_raw(engine)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gist_core_free(engine: *mut GistEngine) {
    if !engine.is_null() {
        unsafe {
            drop(Box::from_raw(engine));
        }
    }
}

#[no_mangle]
pub extern "C" fn gist_core_set_callback(
    engine: *mut GistEngine,
    callback: Option<EventCallback>,
) {
    if engine.is_null() {
        return;
    }
    let eng = unsafe { &*engine };
    let mut cb = eng.event_cb.lock().unwrap();
    *cb = callback;
}

#[no_mangle]
pub extern "C" fn gist_core_add_account(
    engine: *mut GistEngine,
    id: *const c_char,
    name: *const c_char,
    email: *const c_char,
    imap_host: *const c_char,
    imap_port: u16,
    username: *const c_char,
    password: *const c_char,
) -> bool {
    if engine.is_null() || id.is_null() || email.is_null() || imap_host.is_null() {
        return false;
    }

    let eng = unsafe { &*engine };
    let to_str = |p: *const c_char| unsafe { CStr::from_ptr(p).to_string_lossy().into_owned() };

    let acc = Account {
        id: to_str(id),
        name: to_str(name),
        email: to_str(email),
        imap_host: to_str(imap_host),
        imap_port,
        username: to_str(username),
        password: to_str(password),
    };

    eng.storage.save_account(&acc).is_ok()
}

#[no_mangle]
pub extern "C" fn gist_core_sync_account(
    engine: *mut GistEngine,
    account_id: *const c_char,
) -> bool {
    if engine.is_null() || account_id.is_null() {
        return false;
    }

    let eng = unsafe { &*engine };
    let target_id = unsafe { CStr::from_ptr(account_id).to_string_lossy().into_owned() };

    let accounts = match eng.storage.get_accounts() {
        Ok(accs) => accs,
        Err(_) => return false,
    };

    let acc = match accounts.into_iter().find(|a| a.id == target_id) {
        Some(a) => a,
        None => return false,
    };

    let storage_clone = Arc::clone(&eng.storage);
    let cb_clone = Arc::clone(&eng.event_cb);

    thread::spawn(move || {
        let notify = move |msg: &str| {
            let cb_guard = cb_clone.lock().unwrap();
            if let Some(cb) = *cb_guard {
                if let Ok(c_msg) = CString::new(msg) {
                    cb(c_msg.as_ptr());
                }
            }
        };

        if let Err(e) = imap::sync_inbox(&acc, &storage_clone, &notify) {
            notify(&format!(r#"{{"event":"sync_error","error":"{}"}}"#, e));
        }
    });

    true
}

#[no_mangle]
pub extern "C" fn gist_core_get_messages_json(
    engine: *mut GistEngine,
    folder: *const c_char,
) -> *mut c_char {
    if engine.is_null() {
        return std::ptr::null_mut();
    }

    let eng = unsafe { &*engine };
    let folder_str = if folder.is_null() {
        "inbox"
    } else {
        unsafe { CStr::from_ptr(folder).to_str().unwrap_or("inbox") }
    };

    let messages = eng.storage.get_messages(folder_str).unwrap_or_default();
    let json_str = serde_json::to_string(&messages).unwrap_or_else(|_| "[]".to_string());

    match CString::new(json_str) {
        Ok(c_str) => c_str.into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gist_core_free_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe {
            drop(CString::from_raw(s));
        }
    }
}
