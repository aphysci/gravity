use std::ffi::c_char;

use cxx::SharedPtr;

use crate::ffi::{new_future_response, GFutureResponse};




pub struct FutureResponse {
    pub(crate) fr: SharedPtr<GFutureResponse>,
}

impl FutureResponse {
    pub fn with_array(array: &[u8]) -> FutureResponse {
        let ptr = array.as_ptr() as * const c_char;
        let size = array.len() as i32;
        FutureResponse { fr: unsafe { new_future_response(ptr, size) } }
    }

}