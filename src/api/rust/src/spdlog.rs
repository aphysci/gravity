#![allow(dead_code)]
use crate::ffi;


use cxx::{let_cxx_string};






// pub struct GravitySubscriber {
//     subFilled: fn(Vec<GravityDataProduct>),
// }

// impl GravitySubscriber {

// }
pub struct SpdLog {}

impl SpdLog {
    

    pub fn critical(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi::spdlog_critical(&m);
    }
    pub fn error(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi::spdlog_error(&m);
    }
    pub fn warn(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi::spdlog_warn(&m);
    }
    pub fn info(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi::spdlog_info(&m);
    }
    pub fn debug(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi::spdlog_debug(&m);
    }
    pub fn trace(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi::spdlog_trace(&m);
    }
    

}


