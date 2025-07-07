#![allow(dead_code)]
use std::ffi::c_char;

use cxx::{let_cxx_string, UniquePtr};
use crate::ffi::{self, *};

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

    pub(crate) fn from_gdp(gdp: UniquePtr<GDataProduct>) -> GravityDataProduct {
        GravityDataProduct {gdp: gdp}
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
    pub fn get_receieved_timestamp(&self) -> u64 {
        ffi::get_receieved_timestamp(&self.gdp)
    }
    pub fn get_gravity_timestamp(&self) -> u64{
        ffi::get_gravity_timestamp(&self.gdp)
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
    pub fn get_data() {

    }
    pub fn get_data_size() -> i32 {
        0
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
    pub fn get_size(&self) -> i32 {
        0
    }
    pub fn parse_from_array(&self, array: &[u8]) {

    }
    pub fn serialize_to_array(&self, array: &[u8]) -> bool {
        true // think about changing array -> string
    }   
    pub fn get_component_id(&self) -> String {
        String::new()
    }
    pub fn get_domain(&self) -> String {
        String::new()
    }
    pub fn is_future_response(&self) -> bool {
        true
    }
    pub fn is_cached_data_product(&self) -> bool {
        true
    }
    pub fn set_is_cached_data_product(&self, cached: bool) {

    } 
    pub fn get_future_socket_url(&self) -> String {
        String::new()
    }
    pub fn is_relayed_data_product(&self) -> bool {
        true
    }
    pub fn set_is_relayed_data_product(&self, relayed: bool) {

    }
    pub fn set_protocol(&self, protocol: String) {

    }
    pub fn get_protocol(&self) -> String {
        String::new()
    }
    pub fn set_type_name(&self, data_type: String) {

    }
    pub fn get_type_name(&self) -> String {
        String::new()
    }
    pub fn get_registration_time(&self) -> u32 {
        2
    }
}