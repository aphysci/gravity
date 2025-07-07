#![allow(dead_code)]
use std::collections::HashMap;
use cxx::{let_cxx_string, CxxString, CxxVector, SharedPtr, UniquePtr};
use crate::ffi::*;
use crate::{ffi, gravity_subscriber::*,
     gravity_data_product::*, gravity_requestor::*, gravity_service_provider::*};


pub type GravityReturnCode = GReturnCode;
pub type GravityTransportType = GTransportType;

pub struct GravityNode {
    gn: UniquePtr<GNode>,
    cpp_subscriber_map: HashMap<usize, UniquePtr<RustSubscriber>>,
    cpp_service_provider_map: HashMap<usize, UniquePtr<RustServiceProvider>>,
    cpp_requestor_provider_map: HashMap<usize, UniquePtr<RustRequestor>>,
    // rust_subscriber_list: Vec<Box<dyn GravitySubscriber>>,
}

impl GravityNode {
    pub fn new() -> GravityNode {
        GravityNode { 
            gn: ffi::GravityNode(), 
            cpp_subscriber_map: HashMap::new(),
            cpp_service_provider_map: HashMap::new(),
            cpp_requestor_provider_map: HashMap::new(),
        }
    }
    
    pub fn from(component_id: impl AsRef<[u8]>) -> GravityNode {
        let_cxx_string!(cid = component_id);
        GravityNode { 
            gn: ffi::gravity_node_id(&cid), 
            cpp_subscriber_map: HashMap::new(),
            cpp_service_provider_map: HashMap::new(),
            cpp_requestor_provider_map: HashMap::new(),
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
    fn get_addr(subscriber: &impl GravitySubscriber) -> usize {
        let boxed = Box::new(subscriber as &dyn GravitySubscriber);
        let pointer = Box::into_raw(boxed);
        pointer as usize
    }
    pub fn subscribe(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let addr   = GravityNode::get_addr(subscriber);      
        let func = GravityNode::sub_filled_internal;
        let rust_sub = new_rust_subscriber(func, addr);       

        let ret= ffi::subscribe(&self.gn, &dpid, &rust_sub);
        // std::mem::forget(rust_sub);
        let key = subscriber as * const _ as usize;
        self.cpp_subscriber_map.insert(key, rust_sub);
        ret
    }
    pub fn subscribe_filter(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber, filter: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let addr = GravityNode::get_addr(subscriber);
        let func = GravityNode::sub_filled_internal;
        let rust_sub = new_rust_subscriber(func, addr);

        let ret = ffi::subscribe_filter(&self.gn, &dpid, &rust_sub, &f);
        let key = subscriber as * const _ as usize;
        self.cpp_subscriber_map.insert(key, rust_sub);
        ret
    }

    pub fn subscribe_domain(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber, filter: impl AsRef<[u8]>, domain: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        let addr = GravityNode::get_addr(subscriber);
        let func = GravityNode::sub_filled_internal;
        let rust_sub = new_rust_subscriber(func, addr);

        let ret = ffi::subscribe_domain(&self.gn, &dpid, &rust_sub, &f, &d);
        let key = subscriber as * const _ as usize;
        self.cpp_subscriber_map.insert(key, rust_sub);
        ret
    }

    pub fn subscribe_cache(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber, filter: impl AsRef<[u8]>, domain: impl AsRef<[u8]>, recieve_last_cache_value: bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        let addr = GravityNode::get_addr(subscriber);
        let func = GravityNode::sub_filled_internal;
        let rust_sub = new_rust_subscriber(func, addr);

        let ret = ffi::subscribe_cache(&self.gn, &dpid, &rust_sub, &f, &d, recieve_last_cache_value);
        let key = subscriber as * const _ as usize;
        self.cpp_subscriber_map.insert(key, rust_sub);
        ret
    }

    
    pub fn unsubscribe(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let key = subscriber as * const _ as usize;
        let rust_sub_op = self.cpp_subscriber_map.get(&key);
        match rust_sub_op {
            None => GravityReturnCode::SUCCESS,
            Some(rust_sub) => {
                let temp = ffi::unsubscribe(&self.gn, &dpid, rust_sub);
                self.cpp_subscriber_map.remove(&key);
                temp
            }
        }
        

    }

    pub fn request_async(&mut self, service_id:  impl AsRef<[u8]>, data_product: &GravityDataProduct, 
        requestor: &impl GravityRequestor, request_id: Option< impl AsRef<[u8]>>, timeout_milliseconds: Option<i32>,
        domain: Option< impl AsRef<[u8]>>) -> GravityReturnCode
    {
        let_cxx_string!(sid = service_id);
        
        let rid = match request_id {
            Some(r ) => 
                str::from_utf8(r.as_ref()).unwrap().to_string(),
            None => "".to_string(),
        };
        let_cxx_string!(req = rid);
        let timeout = match timeout_milliseconds {
            Some(i) => i,
            None => -1,
        };
        let d= match domain {
            Some(d) => str::from_utf8(d.as_ref()).unwrap().to_string(),              
            None => "".to_string()
        };
        let_cxx_string!(dom = d);
        let boxed = Box::new(requestor as &dyn GravityRequestor);
        let pointer = Box::into_raw(boxed);
        let addr = pointer as usize;
        let func = GravityNode::request_filled_internal;

        let rust_req = ffi::new_rust_requestor(func, addr);
        let ret = ffi::request_async(&self.gn, &sid, &data_product.gdp, &rust_req, &req, timeout, &dom);
        
        let key = requestor as * const _ as usize;
        self.cpp_requestor_provider_map.insert(key, rust_req);
        ret
    
    }

    pub fn request_sync(&self, service_id: impl AsRef<[u8]>, request: &GravityDataProduct) -> Option<GravityDataProduct> {
        let_cxx_string!(sid = service_id);
        let_cxx_string!(domain = "");
        let gdp = ffi::request_sync(&self.gn, &sid, &request.gdp, -1, &domain);
        if gdp.is_null() {
            return None;
        }
        Some(GravityNode::to_rust_gdp(gdp.as_ref().unwrap()))
    }

    pub fn register_service(&mut self, service_id:  impl AsRef<[u8]>, transport_type: GravityTransportType, server: &impl GravityServiceProvider) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        let func = GravityNode::request_internal;
        let boxed = Box::new(server as &dyn GravityServiceProvider);
        let pointer = Box::into_raw(boxed);
        let addr = pointer as usize;

        let rust_server = ffi::new_rust_service_provider(func, addr);
        let ret = ffi::register_service(&self.gn, &sid, transport_type, &rust_server);
        let key = server as * const _ as usize;
        self.cpp_service_provider_map.insert(key, rust_server);
        ret
    }
    pub fn unregister_service(&self, service_id: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        ffi::unregister_service(&self.gn, &sid)
    }


    pub fn register_relay(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber,
                          local_only: bool, transport_type: GravityTransportType) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let addr = GravityNode::get_addr(subscriber);
        let func = GravityNode::sub_filled_internal;
        let rust_sub = new_rust_subscriber(func, addr);
        
        let ret = ffi::register_relay(&self.gn, &dpid, &rust_sub, local_only, transport_type);
        let key = subscriber as * const _ as usize;
        self.cpp_subscriber_map.insert(key, rust_sub);
        ret
    }
    pub fn register_relay_cache(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber,
                                local_only: bool, transport_type: GravityTransportType, cache_last_value: bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let addr = GravityNode::get_addr(subscriber);
        let func = GravityNode::sub_filled_internal;
        let rust_sub = new_rust_subscriber(func, addr);

        let ret = ffi::register_relay_cache(&self.gn, &dpid, &rust_sub, local_only, transport_type, cache_last_value);
        let key = subscriber as *const _ as usize;
        self.cpp_subscriber_map.insert(key, rust_sub);
        ret
    }
    pub fn unregister_relay(&mut self, data_product_id: impl AsRef<[u8]>, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let key = subscriber as *const _ as usize;

        let rust_sub = self.cpp_subscriber_map.get(&key);
        match rust_sub {
            None => GravityReturnCode::SUCCESS,
            Some( sub) => {
                let temp = ffi::unregister_relay(&self.gn, &dpid, sub);
                self.cpp_subscriber_map.remove(&key);
                temp
            }
        }
    }


    fn to_rust_gdp(gdp: &GDataProduct) -> GravityDataProduct{
        GravityDataProduct::from_gdp(ffi::copy_gdp(gdp))
    }


    fn rustify(data_products: &CxxVector<GDataProduct>) -> Vec<GravityDataProduct> {
        let mut v = Vec::new();
            for item in data_products.iter() {
                let to_add = GravityNode::to_rust_gdp(item);
                v.push(to_add);
            }
        v
    }

    fn sub_filled_internal(data_products: &CxxVector<GDataProduct>, addr: usize) {
        let v = GravityNode::rustify(data_products);
        let subscriber = addr as * mut &dyn GravitySubscriber;
        
        let b = unsafe {*subscriber};
        b.subscription_filled(&v);
        
    }

    fn request_filled_internal(service_id: &CxxString, request_id: &CxxString, response: &GDataProduct, addr: usize) {
        let sid = service_id.to_str().unwrap().to_string();
        let rid = request_id.to_str().unwrap().to_string();
        let gdp = GravityNode::to_rust_gdp(response);

        let requestor = addr as * mut &dyn GravityRequestor;
        let b = unsafe {*requestor};
        b.request_filled(sid, rid, &gdp);
    }

    fn request_internal(service_id: &CxxString, data_product: &GDataProduct, addr: usize) -> SharedPtr<GDataProduct>{
        let gdp = GravityNode::to_rust_gdp(data_product);
        let sid = service_id.to_str().unwrap().to_string();

        let p = addr as * mut &dyn GravityServiceProvider;
        let service_provider = unsafe {*p};
        let g = service_provider.request(sid, &gdp);
        ffi::copy_gdp_shared(&g.gdp)
    }
    
}
