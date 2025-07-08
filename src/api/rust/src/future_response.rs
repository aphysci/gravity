use std::ffi::c_char;

use cxx::SharedPtr;

use crate::ffi::{new_future_response, GFutureResponse};




pub struct FutureResponse {
    pub(crate) fr: SharedPtr<GFutureResponse>,
}

impl FutureResponse {
    pub fn from(array: &[u8], size: i32) -> FutureResponse {
        let ptr = array.as_ptr() as * const c_char;
        FutureResponse { fr: unsafe { new_future_response(ptr, size) } }
    }

}