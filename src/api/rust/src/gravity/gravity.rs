#![allow(dead_code)]
use std::ffi::c_char;
use crate::gravity::ffi;

use cxx::{let_cxx_string, UniquePtr, CxxVector};


use crate::gravity::ffi::*;

pub type GravityReturnCode = GReturnCode;
pub type GravityTransportType = GTransportType;

pub struct GravityDataProduct {
    gdp: UniquePtr<GDataProduct>,
}

pub struct GravityNode {
    gn: UniquePtr<GNode>,
    cpp_subscriber_list: Vec<UniquePtr<RustSubscriber>>,
    // rust_subscriber_list: Vec<Box<dyn GravitySubscriber>>,
}

impl GravityNode {
    pub fn new() -> GravityNode {
        GravityNode { 
            gn: ffi::GravityNode(), 
            cpp_subscriber_list: Vec::new(),
            // rust_subscriber_list: Vec::new(),
        }
    }
    
    pub fn from(component_id: impl AsRef<[u8]>) -> GravityNode {
        let_cxx_string!(cid = component_id);
        GravityNode { 
            gn: ffi::gravity_node_ID(&cid), 
            cpp_subscriber_list: Vec::new(),
            // rust_subscriber_list: Vec::new(),
        }
    }

    pub fn init(&self, component_id: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(cid = component_id);
        ffi::init(&self.gn, &cid)
    }

    pub fn init_default(&self) -> GravityReturnCode {
        ffi::init_default(&self.gn)
    }

    pub fn wait_for_exit(&self) {
        ffi::wait_for_exit(&self.gn);
    }
    pub fn publish(&self, data_product: GravityDataProduct) -> GravityReturnCode
    {
        ffi::publish(&self.gn, &(data_product.gdp))
    }
    pub fn subscribers_exist(&self, data_product_id: impl AsRef<[u8]>, has_subscribers: &mut bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        ffi::subsribers_exist(&self.gn, &dpid, has_subscribers)
    }
    pub fn start_heartbeat(&self, interval_in_microseconds: i64) -> GravityReturnCode {
        ffi::start_heartbeat(&self.gn, interval_in_microseconds)
    }
    pub fn stop_heartbeat(&self) -> GravityReturnCode {
        ffi::stop_heartbeat(&self.gn)
    }
    pub fn get_string_param(&self, key: impl AsRef<[u8]>, default_value: impl AsRef<[u8]>) -> String {
        let_cxx_string!(k = key);
        let_cxx_string!(default_val = default_value);
        ffi::get_string_param(&self.gn, &k, &default_val).to_string()
    }
    pub fn get_int_param(&self, key: impl AsRef<[u8]>, default_value: i32) -> i32 {
        let_cxx_string!(k = key);
        ffi::get_int_param(&self.gn, &k, default_value)
    }
    pub fn get_float_param(&self, key: impl AsRef<[u8]>, default_value: f64) -> f64 {
        let_cxx_string!(k = key);
        ffi::get_float_param(&self.gn, &k, default_value)
    }
    pub fn get_bool_param(&self, key: impl AsRef<[u8]>, default_value: bool) -> bool {
        let_cxx_string!(k = key);
        ffi::get_bool_param(&self.gn, &k, default_value)
    }
    pub fn get_component_id(&self) -> String  
    { (*ffi::get_component_ID(&self.gn)).to_str().unwrap().to_string()} 

    pub fn register_data_product(&self, data_product_id: impl AsRef<[u8]>,
         transport_type: GravityTransportType) -> GravityReturnCode
    {
        let_cxx_string!(dpid = data_product_id);
        ffi::register_data_product(&self.gn, &dpid, transport_type)
    }
    pub fn register_data_product_cache(&self, data_product_id: impl AsRef<[u8]>,
         transport_type: GravityTransportType, cache_last_value: bool) -> GravityReturnCode
    {
        let_cxx_string!(dpid = data_product_id);
        ffi::register_data_product_cache(&self.gn, &dpid, transport_type, cache_last_value)
    }
    pub fn unregister_data_product(&self, data_product_id: impl AsRef<[u8]>){
        let_cxx_string!(dpid = data_product_id);
        ffi::unregister_data_product(&self.gn, &dpid);
    }
    pub fn get_code_string(&self, code: GravityReturnCode) -> String {
        ffi::get_code_string(&self.gn, code).to_str().unwrap().to_string()
    }
    pub fn get_ip(&self) -> String {
        ffi::get_IP(&self.gn).to_str().unwrap().to_string()
    }
    pub fn get_domain(&self) -> String {
        ffi::get_domain(&self.gn).to_str().unwrap().to_string()
    }

    pub fn subscribe_temp(&self, data_product_id: impl AsRef<[u8]>, subscriber: &UniquePtr<RustSubscriber>) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        ffi::subscribe(&self.gn, &dpid, subscriber)
    }

    pub fn subscribe(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        // let _func = _subscriber.subscriptionFilled;
        let boxed = Box::new(subscriber as &dyn GravitySubscriber);
        let pointer = Box::into_raw(boxed);
        let addr = pointer as usize;
        
        fn sub_filled_internal(data_products: &CxxVector<GDataProduct>, addr: usize) {
            let v = rustify(data_products);
            let subscriber = addr as * mut &dyn GravitySubscriber;
            
            let b = unsafe {*subscriber};
            b.subscription_filled(&v);
            
        }
        
        let func = sub_filled_internal;
        let rust_sub = newRustSubscriber(func, addr);
        
        

        let ret = self.subscribe_internal(&data_product_id, &rust_sub);
        // std::mem::forget(rust_sub);
        self.cpp_subscriber_list.push(rust_sub);
        
        ret
    }

    fn subscribe_internal(&self, data_product_id: impl AsRef<[u8]>, subscriber: &UniquePtr<RustSubscriber>) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        ffi::subscribe(&self.gn, &dpid, subscriber)
    }
    
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

    fn from_gdp(gdp: UniquePtr<GDataProduct>) -> GravityDataProduct {
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

// pub struct GravitySubscriber {
//     subFilled: fn(Vec<GravityDataProduct>),
// }

// impl GravitySubscriber {

// }
pub struct GravityLogger {}

impl GravityLogger {
    

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


fn to_rust_gdp(gdp: &GDataProduct) -> GravityDataProduct{
    GravityDataProduct::from_gdp(ffi::copy_gdp(gdp))
}


fn rustify(data_products: &CxxVector<GDataProduct>) -> Vec<GravityDataProduct> {
        let mut v = Vec::new();
        for item in data_products.iter() {
            let to_add = to_rust_gdp(item);
            v.push(to_add);
        }
        v
    }

pub trait GravitySubscriber {
    fn subscription_filled(&self, data_products: &Vec<GravityDataProduct>);
}
