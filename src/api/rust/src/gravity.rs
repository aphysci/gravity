use core::ffi;
use std::{default, ffi::c_char, ops::Sub, sync::Arc};
use crate::{ffi1, protos::GravityDataProductPB::GravityDataProductPB};

use cxx::{let_cxx_string, UniquePtr, CxxString, CxxVector};
use protobuf::reflect::MessageDescriptor;
use protobuf::Message;
use autocxx::{generate, prelude::*, safety, subclass::{self, subclass, prelude::*}};

use crate::ffi1::*;



pub struct GravityDataProduct {
    gdp: UniquePtr<GDataProduct>,
}

pub struct GravityNode {
    gn: UniquePtr<GNode>,
}

impl GravityNode {
    pub fn new() -> GravityNode {
        GravityNode { gn: ffi1::GravityNode() }
    }
    
    pub fn from(componentID: impl AsRef<[u8]>) -> GravityNode {
        let_cxx_string!(cid = componentID);
        GravityNode { gn: ffi1::gravity_node_ID(&cid)}
    }

    pub fn init(&self, componentID: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(cid = componentID);
        ffi1::init(&self.gn, &cid)
    }

    pub fn init_default(&self) -> GravityReturnCode {
        ffi1::init_default(&self.gn)
    }

    pub fn wait_for_exit(&self) {
        ffi1::wait_for_exit(&self.gn);
    }
    pub fn publish(&self, dataProduct: GravityDataProduct) -> GravityReturnCode
    {
        ffi1::publish(&self.gn, &(dataProduct.gdp))
    }
    pub fn subscribers_exist(&self, dataProductID: impl AsRef<[u8]>, has_subscribers: &mut bool) -> GravityReturnCode {
        let_cxx_string!(dpid = dataProductID);
        ffi1::subsribers_exist(&self.gn, &dpid, has_subscribers)
    }
    pub fn start_heartbeat(&self, interval_in_microseconds: i64) -> GravityReturnCode {
        ffi1::start_heartbeat(&self.gn, interval_in_microseconds)
    }
    pub fn stop_heartbeat(&self) -> GravityReturnCode {
        ffi1::stop_heartbeat(&self.gn)
    }
    pub fn get_string_param(&self, key: impl AsRef<[u8]>, default_value: impl AsRef<[u8]>) -> String {
        let_cxx_string!(k = key);
        let_cxx_string!(default_val = default_value);
        ffi1::get_string_param(&self.gn, &k, &default_val).to_string()
    }
    pub fn get_int_param(&self, key: impl AsRef<[u8]>, default_value: i32) -> i32 {
        let_cxx_string!(k = key);
        ffi1::get_int_param(&self.gn, &k, default_value)
    }
    pub fn get_float_param(&self, key: impl AsRef<[u8]>, default_value: f64) -> f64 {
        let_cxx_string!(k = key);
        ffi1::get_float_param(&self.gn, &k, default_value)
    }
    pub fn get_bool_param(&self, key: impl AsRef<[u8]>, default_value: bool) -> bool {
        let_cxx_string!(k = key);
        ffi1::get_bool_param(&self.gn, &k, default_value)
    }
    pub fn get_component_ID(&self) -> String  
    { (*ffi1::get_component_ID(&self.gn)).to_str().unwrap().to_string()} 

    pub fn register_data_product(&self, dataProductID: impl AsRef<[u8]>,
         transportType: GravityTransportType) -> GravityReturnCode
    {
        let_cxx_string!(dpid = dataProductID);
        ffi1::register_data_product(&self.gn, &dpid, transportType)
    }
    pub fn register_data_product_cache(&self, dataProductID: impl AsRef<[u8]>,
         transportType: GravityTransportType, cacheLastValue: bool) -> GravityReturnCode
    {
        let_cxx_string!(dpid = dataProductID);
        ffi1::register_data_product_cache(&self.gn, &dpid, transportType, cacheLastValue)
    }
    pub fn unregister_data_product(&self, dataProductID: impl AsRef<[u8]>){
        let_cxx_string!(dpid = dataProductID);
        ffi1::unregister_data_product(&self.gn, &dpid);
    }
    pub fn get_code_string(&self, code: GravityReturnCode) -> String {
        ffi1::get_code_string(&self.gn, code).to_str().unwrap().to_string()
    }
    pub fn get_IP(&self) -> String {
        ffi1::get_IP(&self.gn).to_str().unwrap().to_string()
    }
    pub fn get_domain(&self) -> String {
        ffi1::get_domain(&self.gn).to_str().unwrap().to_string()
    }

    pub fn subscribe_temp(&self, dataProductID: impl AsRef<[u8]>, subscriber: &UniquePtr<RustSubscriber>) -> GravityReturnCode {
        let_cxx_string!(dpid = dataProductID);
        ffi1::subscribe(&self.gn, &dpid, subscriber)
    }

    pub fn subscribe(&self, dataProductID: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = dataProductID);

        fn subFilledInternal(dataProducts: &CxxVector<GDataProduct>) {
            let _v = rustify(dataProducts);

        }
        let func = subFilledInternal;
        let rust_sub = newRustSubscriber(func);
        ffi1::subscribe(&self.gn, &dpid, &rust_sub);
        GravityReturnCode::SUCCESS
    }
    
}

impl GravityDataProduct {
    pub fn new() -> GravityDataProduct {
        GravityDataProduct { gdp: ffi1::gravity_data_product_default() }
    }

    pub fn from_id(dataProductId: impl AsRef<[u8]>) -> GravityDataProduct {
        let_cxx_string!(dpid = dataProductId);
        GravityDataProduct {gdp: ffi1::gravity_data_product(&dpid)}
    }

    fn from_array(array: &[u8], size: i32) -> GravityDataProduct {
        let arrayPtr = array as *const _ as * const c_char;
        GravityDataProduct {gdp: unsafe {ffi1::gravity_data_product_bytes(arrayPtr, size)}}
    }

    fn from_gdp(gdp: UniquePtr<GDataProduct>) -> GravityDataProduct {
        GravityDataProduct {gdp: gdp}
    }

    pub fn set_data_basic(&self, data: &str, size:i32) {
        let d = data as *const _ as *const c_char;
        unsafe { ffi1::set_data_basic(&self.gdp, d, size); }
    }
    pub fn set_data(&self, data: &impl protobuf::Message) {
        let v = data.write_to_bytes().unwrap();
        let bytes = v.as_ptr() as *const c_char;

        unsafe {ffi1::set_data(&self.gdp, bytes, data.compute_size() as i32);}
    }
    pub fn get_gravity_timestamp(&self) -> u64{
        ffi1::get_gravity_timestamp(&self.gdp)
    }
    pub fn get_receieved_timestamp(&self) -> u64 {
        ffi1::get_receieved_timestamp(&self.gdp)
    }
    pub fn get_data_product_ID(&self) -> String {
        ffi1::get_data_product_ID(&self.gdp).to_str().unwrap().to_string()
    }
    pub fn set_software_version(&self, softwareVersion: impl AsRef<[u8]>)
    {
        let_cxx_string!(sv = softwareVersion);
        ffi1::set_software_version(&self.gdp, &sv);
    }
    pub fn get_software_version(&self) -> String {
        ffi1::get_software_version(&self.gdp).to_str().unwrap().to_string()
    }
    pub fn populate_message(&self, data: &mut impl protobuf::Message) -> bool{
        let data_str = ffi1::get_proto_data(&self.gdp);
        let bytes = data_str.as_bytes();
        // let smth = GravityDataProductPB::parse_from_bytes(bytes).unwrap();
        data.merge_from_bytes(bytes).unwrap();
        true

        

    }
}

pub struct gravity_logger {}

impl gravity_logger {
    

    pub fn critical(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi1::spdlog_critical(&m);
    }
    pub fn error(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi1::spdlog_error(&m);
    }
    pub fn warn(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi1::spdlog_warn(&m);
    }
    pub fn info(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi1::spdlog_info(&m);
    }
    pub fn debug(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi1::spdlog_debug(&m);
    }
    pub fn trace(message: impl AsRef<[u8]>) {
        let_cxx_string!(m = message);
        ffi1::spdlog_trace(&m);
    }
    

}


fn to_rust_gdp(gdp: &GDataProduct) -> GravityDataProduct{
    GravityDataProduct::from_gdp(ffi1::copy_gdp(gdp))
}

pub trait GravitySubscriber {
    fn subscriptionFilled(dataProducts: Vec<GravityDataProduct>);

    
}
pub fn rustify(dataProducts: &CxxVector<GDataProduct>) -> Vec<GravityDataProduct> {
        let mut v = Vec::new();
        for item in dataProducts.iter() {
            let to_add = to_rust_gdp(item);
            v.push(to_add);
        }
        v
    }
// trait SuperClass {
//     fn subscriptionFilled(dataProducts: Vec<GravityDataProduct>);
// }

// trait Subclass : SuperClass {
//      fn subscriptionFilledInternal(&self, dataProducts: &CxxVector<GDataProduct>)
//     {
//      let mut v = Vec::new();
//      for item in dataProducts.iter() {
//          let to_add = to_rust_gdp(item);
//          v.push(to_add);
//      }   
//      // subscriber.
//     subscriptionFilled(v);
// }   
// }



// pub trait GravitySubscriber {
//     fn subscriptionFilled(dataProducts: &Vec<GravityDataProduct>);
// }


// pub struct GSubscriber {
//     subscriber: impl GravitySubsriber_methods,
// }