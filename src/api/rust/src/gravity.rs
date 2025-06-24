use std::ffi::c_char;
use crate::{ffi};

use cxx::{let_cxx_string, UniquePtr};
use protobuf::reflect::MessageDescriptor;
use autocxx::{generate, prelude::*, safety, subclass::{self, subclass, prelude::*}};

use crate::ffi::*;



pub struct GraDataProduct {
    gdp: UniquePtr<GravityDataProduct>,
}

pub struct GravityNode {
    gn: UniquePtr<GNode>,
}

impl GravityNode {
    pub fn new() -> GravityNode {
        GravityNode { gn: ffi::GravityNode() }
    }
    
    pub fn new_id(componentID: impl AsRef<[u8]>) -> GravityNode {
        let_cxx_string!(cid = componentID);
        GravityNode { gn: ffi::GravityNodeID(&cid)}
    }

    pub fn init(&self, componentID: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(cid = componentID);
        ffi::init(&self.gn, &cid)
    }

    pub fn init_default(&self) -> GravityReturnCode {
        ffi::init_default(&self.gn)
    }

    pub fn wait_for_exit(&self) {
        ffi::wait_for_exit(&self.gn);
    }
    pub fn getComponentID(&self) -> String  
    { (*ffi::getComponentID(&self.gn)).to_str().unwrap().to_string()} 

    pub fn registerDataProduct(&self, dataProductID: impl AsRef<[u8]>, transportType: GravityTransportType) -> GravityReturnCode
    {
        let_cxx_string!(dpid = dataProductID);
        ffi::registerDataProduct(&self.gn, &dpid, transportType)
    }
    pub fn publish(&self, dataProduct: GraDataProduct) -> GravityReturnCode
    {
        ffi::publish(&self.gn, &(dataProduct.gdp))
    }
    pub fn subscribers_exist(&self, dataProductID: impl AsRef<[u8]>, has_subscribers: &mut bool) -> GravityReturnCode {
        let_cxx_string!(dpid = dataProductID);
        ffi::subsribers_exist(&self.gn, &dpid, has_subscribers)
    }
    
    // pub fn subscribe(&self, dataProductID: impl AsRef<[u8]>, 
    //     subscriber: &RustSubscriber) -> GravityReturnCode {
    //         let_cxx_string!(dpid = dataProductID);
    //         ffi1::subscribe(&self.gn, &dpid, subscriber)

    // }
}

impl GraDataProduct {
    pub fn new(dataProductId: impl AsRef<[u8]>) ->GraDataProduct {
        let_cxx_string!(dpid = dataProductId);
        GraDataProduct {gdp: ffi::GravityDataProduct(&dpid)}
    }

    pub fn setDataBasic(&self, data: &str, size:i32) {
        let d = data as *const _ as *const c_char;
        unsafe { ffi::setDataBasic(&self.gdp, d, size); }
    }
    pub fn setData(&self, data: &impl protobuf::Message) {
        let v = data.write_to_bytes().unwrap();
        let bytes = v.as_ptr() as *const c_char;

        unsafe {ffi::setData(&self.gdp, bytes, data.compute_size() as i32);}
    }
}

pub struct spdlog {}

impl spdlog {
    

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


// pub struct GSubscriber {
//     subscriber: impl GravitySubsriber_methods,
// }