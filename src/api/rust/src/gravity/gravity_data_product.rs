#![allow(dead_code)]
use std::ffi::c_char;

use cxx::{let_cxx_string, UniquePtr};
use crate::gravity::ffi::{self, *};

pub struct GravityDataProduct {
    pub(crate) gdp: UniquePtr<GDataProduct>,
}

impl GravityDataProduct {
    pub fn new() -> GravityDataProduct {
        GravityDataProduct { gdp: ffi::gravity_data_product_default() }
    }

    pub fn from_id(data_product_id: impl AsRef<[u8]>) -> GravityDataProduct {
        let_cxx_string!(dpid = data_product_id);
        GravityDataProduct {gdp: ffi::gravity_data_product(&dpid)}
    }

    fn from_array(array: &[u8], size: i32) -> GravityDataProduct {
        let array_ptr = array as *const _ as * const c_char;
        GravityDataProduct {gdp: unsafe {ffi::gravity_data_product_bytes(array_ptr, size)}}
    }

    pub fn from_gdp(gdp: UniquePtr<GDataProduct>) -> GravityDataProduct {
        GravityDataProduct {gdp: gdp}
    }

    pub fn set_data_basic(&self, data: &str, size:i32) {
        let d = data as *const _ as *const c_char;
        unsafe { ffi::set_data_basic(&self.gdp, d, size); }
    }
    pub fn set_data(&self, data: &impl protobuf::Message) {
        let v = data.write_to_bytes().unwrap();
        let bytes = v.as_ptr() as *const c_char;

        unsafe {ffi::set_data(&self.gdp, bytes, data.compute_size() as i32);}
    }
    pub fn get_gravity_timestamp(&self) -> u64{
        ffi::get_gravity_timestamp(&self.gdp)
    }
    pub fn get_receieved_timestamp(&self) -> u64 {
        ffi::get_receieved_timestamp(&self.gdp)
    }
    pub fn get_data_product_id(&self) -> String {
        ffi::get_data_product_ID(&self.gdp).to_str().unwrap().to_string()
    }
    pub fn set_software_version(&self, software_version: impl AsRef<[u8]>)
    {
        let_cxx_string!(sv = software_version);
        ffi::set_software_version(&self.gdp, &sv);
    }
    pub fn get_software_version(&self) -> String {
        ffi::get_software_version(&self.gdp).to_str().unwrap().to_string()
    }
    pub fn populate_message(&self, data: &mut impl protobuf::Message) -> bool{
        let data_str = ffi::get_proto_data(&self.gdp);
        let mut pointer = data_str as * const c_char as * const u8;
        
        let size = ffi::get_data_size(&self.gdp);

        
        let mut bytes_buf = Vec::new();
        for _ in 0..size {
            unsafe {
                bytes_buf.push(*pointer);
                pointer = pointer.offset(1);
            }
            
        }
        let bytes = bytes_buf.as_slice();
        // let smth = GravityDataProductPB::parse_from_bytes(bytes).unwrap();
        data.merge_from_bytes(bytes).unwrap();
        true

        

    }
}