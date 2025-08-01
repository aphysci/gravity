#![allow(dead_code)]
use std::collections::HashMap;
use cxx::{let_cxx_string, CxxString, CxxVector, SharedPtr, UniquePtr};
use crate::ffi::*;
use crate::gravity_heartbeat_listener::GravityHeartbeatListener;
use crate::gravity_subscription_monitor::GravitySubscriptionMonitor;
use crate::{ffi, gravity_subscriber::*,
     gravity_data_product::*, gravity_requestor::*, gravity_service_provider::*
    , future_response::*};

/// Return codes used on a Gravity system.
pub type GravityReturnCode = GravityReturnCodes;

/// Network transport protocols available on a Gravity system.
pub type GravityTransportType = GravityTransportTypes;


/// A component that provides a simple interface point to a Gravity-enabled application.
pub struct GravityNode {
    gn: UniquePtr<GNode>,
    cpp_subscriber_map: HashMap<usize, (UniquePtr<RustSubscriber>, usize)>,
    cpp_service_provider_map: HashMap<usize, (UniquePtr<RustServiceProvider>, usize)>,
    cpp_requestor_provider_map: HashMap<usize, (UniquePtr<RustRequestor>, usize)>,
    cpp_listener_map: HashMap<usize, (UniquePtr<RustHeartbeatListener>, usize, String, String)>,
    cpp_monitor_map: HashMap<usize, (UniquePtr<RustSubscriptionMonitor>, usize)>,
    // rust_subscriber_list: Vec<Box<dyn GravitySubscriber>>,
}

impl GravityNode {
    /// Default constructor.
    pub fn new() -> GravityNode {
        GravityNode { 
            gn: ffi::GravityNode(), 
            cpp_subscriber_map: HashMap::new(),
            cpp_service_provider_map: HashMap::new(),
            cpp_requestor_provider_map: HashMap::new(),
            cpp_listener_map: HashMap::new(),
            cpp_monitor_map: HashMap::new(),
        }
    }
    
    /// Constructor that also initializes.
    /// Uses component_id to initialize.
    pub fn with_id(component_id: &str) -> GravityNode {
        let_cxx_string!(cid = component_id);
        GravityNode { 
            gn: ffi::gravity_node_id(&cid), 
            cpp_subscriber_map: HashMap::new(),
            cpp_service_provider_map: HashMap::new(),
            cpp_requestor_provider_map: HashMap::new(),
            cpp_listener_map: HashMap::new(),
            cpp_monitor_map: HashMap::new(),
        }
    }

    /// Initialize the Gravity infrastructure.
    pub fn init(&self, component_id: &str) -> GravityReturnCode {
        let_cxx_string!(cid = component_id);
        ffi::init(&self.gn, &cid)
    }

    /// Initialize the Gravity infrastructure.
    /// Reads the component_id from the Gravity.ini file.
    pub fn init_default(&self) -> GravityReturnCode {
        ffi::init_default(&self.gn)
    }

    /// Wait for the GravityNode to exit
    pub fn wait_for_exit(&self) {
        ffi::wait_for_exit(&self.gn);
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriper is a type that implements the GravitySubscriber trait.
    pub fn subscribe(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);

        let key = subscriber as * const _ as usize;
        let func = GravityNode::sub_filled_internal;
        let item = self.cpp_subscriber_map.get(&key);

        match item {
            None => {
                let addr   = GravityNode::get_addr(subscriber);      
                let rust_sub = new_rust_subscriber(func, addr);       

                let ret= ffi::subscribe(&self.gn, &dpid, &rust_sub);
                self.cpp_subscriber_map.insert(key, (rust_sub, addr));
                ret
            },
            Some ((rust_sub, _)) => {
                ffi::subscribe(&self.gn, &dpid, rust_sub)
            }
        }
    
        
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriper is a type that implements the GravitySubscriber trait.
    /// Adds optional parameter filter, a text filter to apply to subscription.
    pub fn subscribe_with_filter(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber, filter: &str) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let key = subscriber as * const _ as usize;
        let item = self.cpp_subscriber_map.get(&key);
        let func = GravityNode::sub_filled_internal;
    

        match item {
            None => {
                let addr   = GravityNode::get_addr(subscriber);      
                let rust_sub = new_rust_subscriber(func, addr);       

                let ret= ffi::subscribe_filter(&self.gn, &dpid, &rust_sub, &f);
                self.cpp_subscriber_map.insert(key, (rust_sub, addr));
                ret
            },
            Some ((rust_sub, _)) => {
                ffi::subscribe_filter(&self.gn, &dpid, rust_sub, &f)
            }
        }
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriper is a type that implements the GravitySubscriber trait.
    /// Adds optional parameter filter, a text filter to apply to subscription.
    /// Adds optional parameter domain, the domain of the network components.
    pub fn subscribe_with_domain(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber, filter: &str, domain: &str) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        let key = subscriber as * const _ as usize;
        let item = self.cpp_subscriber_map.get(&key);
        let func = GravityNode::sub_filled_internal;
    

        match item {
            None => {
                let addr   = GravityNode::get_addr(subscriber);      
                let rust_sub = new_rust_subscriber(func, addr);       

                let ret= ffi::subscribe_domain(&self.gn, &dpid, &rust_sub, &f, &d);
                self.cpp_subscriber_map.insert(key, (rust_sub, addr));
                ret
            },
            Some ((rust_sub, _)) => {
                ffi::subscribe_domain(&self.gn, &dpid, rust_sub, &f, &d)
            }
        }
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriper is a type that implements the GravitySubscriber trait.
    /// Adds optional parameter filter, a text filter to apply to subscription.
    /// Adds optional parameter domain, the domain of the network components.
    /// Adds optional paramter receive_last_cache_value.
    pub fn subscribe_with_cache(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber, filter: &str, domain: &str, recieve_last_cache_value: bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        let key = subscriber as * const _ as usize;
        let item = self.cpp_subscriber_map.get(&key);
        let func = GravityNode::sub_filled_internal;
    

        match item {
            None => {
                let addr   = GravityNode::get_addr(subscriber);      
                let rust_sub = new_rust_subscriber(func, addr);       

                let ret= ffi::subscribe_cache(&self.gn, &dpid, &rust_sub, &f, &d, recieve_last_cache_value);
                self.cpp_subscriber_map.insert(key, (rust_sub, addr));
                ret
            },
            Some ((rust_sub, _)) => {
                ffi::subscribe_cache(&self.gn, &dpid, rust_sub, &f, &d, recieve_last_cache_value)
            }
        }
    }
    
    /// Un-subscribe from a data product.
    pub fn unsubscribe(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let key = subscriber as * const _ as usize;
        let rust_sub_op = self.cpp_subscriber_map.get(&key);
        match rust_sub_op {
            None => GravityReturnCode::SUCCESS,
            Some((rust_sub, addr)) => {
                let temp = ffi::unsubscribe(&self.gn, &dpid, rust_sub);
                let pointer = *addr as * mut &dyn GravitySubscriber;
                let _ = unsafe {Box::from_raw(pointer)};
                self.cpp_subscriber_map.remove(&key);
                temp
            }
        }
    }

    /// Publish a data product to the Gravity Service Directory.
    pub fn publish(&self, data_product: &GravityDataProduct) -> GravityReturnCode
    {
        ffi::publish(&self.gn, &data_product.gdp)
    }

    /// Publish a data product to the Gravity Service Directory.
    /// Adds parameter filter_text, a text filter associated with the publish.
    pub fn publish_with_filter_text(&self, data_product: &GravityDataProduct, filter_text: &str) -> GravityReturnCode {
        let_cxx_string!(ft = filter_text);
        ffi::publish_filter(&self.gn, &data_product.gdp, &ft)
    }

    /// Publish a data product to the Gravity Service Directory.
    /// Adds parameter filter_text, a text filter associated with the publish.
    /// Adds paramter timestamp, the time the data product was created.
    pub fn publish_with_timestamp(&self, data_product: &GravityDataProduct, filter_text: &str, timestamp: u64) -> GravityReturnCode {
        let_cxx_string!(ft = filter_text);
        ffi::publish_timestamp(&self.gn, &data_product.gdp, &ft, timestamp)
    }

    /// Determine whether there are any subscribers for this DataProduct.
    pub fn subscribers_exist(&self, data_product_id: &str, has_subscribers: &mut bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        ffi::subsribers_exist(&self.gn, &dpid, has_subscribers)
    }

    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor must be a type that implements GravityRequestor
    /// Returns success flag
    pub fn request_async(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &impl GravityRequestor) -> GravityReturnCode {
        
        let_cxx_string!(sid = service_id);

        let key = requestor as * const _ as usize;
        let item = self.cpp_requestor_provider_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(requestor as &dyn GravityRequestor);
                let pointer = Box::into_raw(boxed);

                let addr = pointer as usize;
                let filled = GravityNode::request_filled_internal;
                let tfunc = GravityNode::request_timeout_internal;
                let rust_req = ffi::new_rust_requestor(filled, tfunc, addr);
                let ret = ffi::request_async(&self.gn, &sid, &request.gdp, &rust_req);


                self.cpp_requestor_provider_map.insert(key, (rust_req, addr));
                ret
            }
            Some((rust_req, _)) => {
                ffi::request_async(&self.gn, &sid, &request.gdp, rust_req)
            }
        }
    }
    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor must be a type that implements GravityRequestor
    /// With parameter request_id, the identifier for this request.
    /// Returns success flag.
    pub fn request_async_with_request_id(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &impl GravityRequestor, request_id: &str) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        let_cxx_string!(req = request_id);

        let key = requestor as * const _ as usize;
        let item = self.cpp_requestor_provider_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(requestor as &dyn GravityRequestor);
                let pointer = Box::into_raw(boxed);

                let addr = pointer as usize;
                let filled = GravityNode::request_filled_internal;
                let tfunc = GravityNode::request_timeout_internal;
                let rust_req = ffi::new_rust_requestor(filled, tfunc, addr);
                let ret = ffi::request_async_request_id(&self.gn, &sid, &request.gdp, &rust_req, &req);


                self.cpp_requestor_provider_map.insert(key, (rust_req, addr));
                ret
            }
            Some((rust_req, _)) => {
                ffi::request_async_request_id(&self.gn, &sid, &request.gdp, rust_req, &req)
            }
        }
    }

    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor must be a type that implements GravityRequestor
    /// With parameter request_id, the identifier for this request.
    /// With parameter timeout_milliseconds, the timeout in Milliseconds (-1 for no timeout).
    /// Returns success flag.
    pub fn request_async_with_timeout(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &impl GravityRequestor, request_id: &str, timeout_milliseconds: i32) -> GravityReturnCode {
            
        let_cxx_string!(sid = service_id);
        let_cxx_string!(req = request_id);

        let key = requestor as * const _ as usize;
        let item = self.cpp_requestor_provider_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(requestor as &dyn GravityRequestor);
                let pointer = Box::into_raw(boxed);

                let addr = pointer as usize;
                let filled = GravityNode::request_filled_internal;
                let tfunc = GravityNode::request_timeout_internal;
                let rust_req = ffi::new_rust_requestor(filled, tfunc, addr);
                let ret = ffi::request_async_timeout(&self.gn, &sid, &request.gdp, &rust_req, &req, timeout_milliseconds);


                self.cpp_requestor_provider_map.insert(key, (rust_req, addr));
                ret
            }
            Some((rust_req, _)) => {
                ffi::request_async_timeout(&self.gn, &sid, &request.gdp, rust_req, &req, timeout_milliseconds)
            }
        }
    }

    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor must be a type that implements GravityRequestor
    /// With parameter request_id, the identifier for this request.
    /// With parameter timeout_milliseconds, the timeout in Milliseconds (-1 for no timeout).
    /// With parameter domain, the domain of the network components.
    /// Returns success flag.
    pub fn request_async_with_domain(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &impl GravityRequestor, request_id: &str, timeout_milliseconds: i32, domain: &str) -> GravityReturnCode
    {
        let_cxx_string!(sid = service_id);
        let_cxx_string!(req = request_id);
        let_cxx_string!(dom = domain);

        let key = requestor as * const _ as usize;
        let item = self.cpp_requestor_provider_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(requestor as &dyn GravityRequestor);
                let pointer = Box::into_raw(boxed);

                let addr = pointer as usize;
                let filled = GravityNode::request_filled_internal;
                let tfunc = GravityNode::request_timeout_internal;
                let rust_req = ffi::new_rust_requestor(filled, tfunc, addr);
                let ret = ffi::request_async_domain(&self.gn, &sid, &request.gdp, &rust_req, &req, timeout_milliseconds, &dom);


                self.cpp_requestor_provider_map.insert(key, (rust_req, addr));
                ret
            }
            Some((rust_req, _)) => {
                ffi::request_async_domain(&self.gn, &sid, &request.gdp, rust_req, &req, timeout_milliseconds, &dom)
            }
        }
    
    }

    /// Makes a synchronous request against a service provider.
    /// Returns None upon failure, Some(GravityDataProduct), where the GravityDataProduct is the data product
    /// representation of the response.
    pub fn request_sync(&self, service_id: &str, request: &GravityDataProduct) -> Option<GravityDataProduct> {
        let_cxx_string!(sid = service_id);
        let gdp = ffi::request_sync(&self.gn, &sid, &request.gdp);
        if gdp.is_null() {
            return None;
        }
        Some(GravityNode::to_rust_gdp(gdp.as_ref().unwrap()))
    }

    /// Makes a synchronous request against a service provider.
    /// With parameter timeout_milliseconds, the timeout in milliseconds (-1 for no timeout).
    /// Returns None upon failure, Some(GravityDataProduct), where the GravityDataProduct is the data product
    /// representation of the response.
    pub fn request_sync_with_timeout(&self, service_id: &str, request: &GravityDataProduct, timeout_milliseconds: i32) -> Option<GravityDataProduct> {
        let_cxx_string!(sid = service_id);
        let gdp = ffi::request_sync_timeout(&self.gn, &sid, &request.gdp, timeout_milliseconds);
        if gdp.is_null() {
            return None;
        }
        Some(GravityNode::to_rust_gdp(gdp.as_ref().unwrap()))
    }
   
    /// Makes a synchronous request against a service provider.
    /// With parameter timeout_milliseconds, the timeout in milliseconds (-1 for no timeout).
    /// With parameter domain, the domain of the network components
    /// Returns None upon failure, Some(GravityDataProduct), where the GravityDataProduct is the data product
    /// representation of the response.
    pub fn request_sync_with_domain(&self, service_id: &str, request: &GravityDataProduct, timeout_milliseconds: i32, domain: &str) -> Option<GravityDataProduct> {
        let_cxx_string!(sid = service_id);
        let_cxx_string!(d = domain);
        let gdp = ffi::request_sync_domain(&self.gn, &sid, &request.gdp, timeout_milliseconds, &d);
        if gdp.is_null() {
            return None;
        }
        Some(GravityNode::to_rust_gdp(gdp.as_ref().unwrap()))
    }

    /// Starts a heart beat for this GravityNode.
    /// Returns success flag.
    pub fn start_heartbeat(&self, interval_in_microseconds: i64) -> GravityReturnCode {
        ffi::start_heartbeat(&self.gn, interval_in_microseconds)
    }

    /// Stops the heart beat for this GravityNode
    /// Returns success flag.
    pub fn stop_heartbeat(&self) -> GravityReturnCode {
        ffi::stop_heartbeat(&self.gn)
    }

    /// Gravity.ini parsing function
    /// key is which item in the Gravity.ini will contain the value.
    /// default_value is what it should return if the key cannot be found.
    pub fn get_string_param(&self, key: &str, default_value: &str) -> String {
        let_cxx_string!(k = key);
        let_cxx_string!(default_val = default_value);
        ffi::get_string_param(&self.gn, &k, &default_val).to_string()
    }

    /// Gravity.ini parsing function
    /// key is which item in the Gravity.ini will contain the value.
    /// default_value is what it should return if the key cannot be found.
    pub fn get_int_param(&self, key: &str, default_value: i32) -> i32 {
        let_cxx_string!(k = key);
        ffi::get_int_param(&self.gn, &k, default_value)
    }

    /// Gravity.ini parsing function
    /// key is which item in the Gravity.ini will contain the value.
    /// default_value is what it should return if the key cannot be found.
    pub fn get_float_param(&self, key: &str, default_value: f64) -> f64 {
        let_cxx_string!(k = key);
        ffi::get_float_param(&self.gn, &k, default_value)
    }

    /// Gravity.ini parsing function
    /// key is which item in the Gravity.ini will contain the value.
    /// default_value is what it should return if the key cannot be found.
    pub fn get_bool_param(&self, key: &str, default_value: bool) -> bool {
        let_cxx_string!(k = key);
        ffi::get_bool_param(&self.gn, &k, default_value)
    }

    /// Get the ID of this gravity node (given in the init function).
    pub fn component_id(&self) -> String  
    { (*ffi::get_component_ID(&self.gn)).to_str().unwrap().to_string()} 

    
    /// Register a data product with the Gravity, and optionally, the Service Directrory, 
    /// making it available to the rest of the Gravity-enabled system.
    /// Returns success flag.
    pub fn register_data_product(&self, data_product_id: &str,
         transport_type: GravityTransportType) -> GravityReturnCode
    {
        let_cxx_string!(dpid = data_product_id);
        ffi::register_data_product(&self.gn, &dpid, transport_type)
    }

    /// Register a data product with the Gravity infrastructure, 
    /// making it available to the rest of the Gravity-enabled system.
    /// With parameter cache_last_value, a flag used to signify whether or not the GravityNode
    /// will cache the last sent value for a published data product.
    /// Returns success flag.
    pub fn register_data_product_with_cache(&self, data_product_id: &str,
         transport_type: GravityTransportType, cache_last_value: bool) -> GravityReturnCode
    {
        let_cxx_string!(dpid = data_product_id);
        ffi::register_data_product_cache(&self.gn, &dpid, transport_type, cache_last_value)
    }

    /// Un-register a data product, resulting in its removal from the Gravity Service Directory.
    pub fn unregister_data_product(&self, data_product_id: &str) -> GravityReturnCode{
        let_cxx_string!(dpid = data_product_id);
        ffi::unregister_data_product(&self.gn, &dpid)
    }

    /// Registers as a service provider with Gravity.
    /// Returns success flag.
    pub fn register_service(&mut self, service_id:  &str, transport_type: GravityTransportType, server: &impl GravityServiceProvider) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        let func = GravityNode::request_internal;
        let key = server as * const _ as usize;
        let item = self.cpp_service_provider_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(server as &dyn GravityServiceProvider);
                let pointer = Box::into_raw(boxed);
                let addr = pointer as usize;

                let rust_server = ffi::new_rust_service_provider(func, addr);
                let ret = ffi::register_service(&self.gn, &sid, transport_type, &rust_server);

                self.cpp_service_provider_map.insert(key, (rust_server, addr));
                ret
            },
            Some((rust_server, _)) => {
                ffi::register_service(&self.gn, &sid, transport_type, rust_server)
            }
        }
        
    }

    // Unregister as a service provider with the Gravity Service Directory.
    // Returns success flag.
    pub fn unregister_service(&self, service_id: &str) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        ffi::unregister_service(&self.gn, &sid)
    }

    /// Registers a callback to be called when we don't get a heartbeat from another component.
    /// Returns success flag.
    pub fn register_heartbeat_listener(&mut self, component_id: &str, interval_in_microseconds: i64, listener: &impl GravityHeartbeatListener) -> GravityReturnCode{
        let_cxx_string!(cid = component_id);

        let key = listener as * const _ as usize;
        let item = self.cpp_listener_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(listener as &dyn GravityHeartbeatListener);
                let pointer = Box::into_raw(boxed);
                let addr = pointer as usize;

                let missed = GravityNode::missed_heartbeat_internal;
                let received = GravityNode::received_heartbeat_internal;

                let rust_listener = ffi::new_rust_heartbeat_listener(missed, received, addr);
                let ret = ffi::register_heartbeat_listener(&self.gn, &cid, interval_in_microseconds, &rust_listener);

                self.cpp_listener_map.insert(key, (rust_listener, addr, String::from(component_id), String::from("")));

                ret  
            },
            Some((rust_listener, _,_,_)) => {
                ffi::register_heartbeat_listener(&self.gn, &cid, interval_in_microseconds, rust_listener)
            }
        }    
    }

    /// Registers a callback to be called when we don't get a heartbeat from another component.
    /// With paramter domain, the name of the domain for the component_id.
    /// Returns success flag.
    pub fn register_heartbeat_listener_with_domain(&mut self, component_id: &str, interval_in_microseconds: i64,
                                        listener: &impl GravityHeartbeatListener, domain: &str) -> GravityReturnCode
    {
        let_cxx_string!(cid = component_id);
        let_cxx_string!(d = domain);

        let key = listener as * const _ as usize;
        let item = self.cpp_listener_map.get(&key);

        match item {
            None => {
                let boxed = Box::new(listener as &dyn GravityHeartbeatListener);
                let pointer = Box::into_raw(boxed);
                let addr = pointer as usize;

                let missed = GravityNode::missed_heartbeat_internal;
                let received = GravityNode::received_heartbeat_internal;

                let rust_listener = ffi::new_rust_heartbeat_listener(missed, received, addr);
                let ret = ffi::register_heartbeat_listener_domain(&self.gn, &cid, interval_in_microseconds, &rust_listener, &d);

                self.cpp_listener_map.insert(key, (rust_listener, addr, String::from(component_id), String::from(domain)));

                ret  
            },
            Some((rust_listener, _,_,_)) => {
                ffi::register_heartbeat_listener_domain(&self.gn, &cid, interval_in_microseconds, rust_listener, &d)
            }
        }
         
    }

    /// Unregisters a callback for when we get a heartbeat from another component.
    /// Returns success flag.
    pub fn unregister_heartbeat_listener(&self, component_id: &str) -> GravityReturnCode {
        let_cxx_string!(cid = component_id);
        ffi::unregister_heartbeat_listener(&self.gn, &cid)
    }

    /// Unregisters a callback for when we get a heartbeat from another component.
    /// With paramter domain, the name of the domain for the component_id.
    /// Returns success flag.
    pub fn unregister_heartbeat_listener_with_domain(&self, component_id: &str, domain: &str) -> GravityReturnCode {
        let_cxx_string!(cid = component_id);
        let_cxx_string!(d = domain);
        ffi::unregister_heartbeat_listener_domain(&self.gn, &cid, &d)
    }

    /// Register a relay that will act as a pass-through for the given data_product_id. It will be 
    /// a publisher and subscriber for the given data_product_id, but other components will only subscribe 
    /// to this data if they are on the same host (local__only == true), or it it is acting as a global relay (local_only == false).
    /// The Gravity infrastructure automatically handles which components should receive relayed or non-relayed data.
    /// 
    /// Returns success flag.
    pub fn register_relay(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber,
                          local_only: bool, transport_type: GravityTransportType) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let key = subscriber as * const _ as usize;
        let item = self.cpp_subscriber_map.get(&key);

        match item {
            None => {
                let addr = GravityNode::get_addr(subscriber);
                let func = GravityNode::sub_filled_internal;
                let rust_sub = new_rust_subscriber(func, addr);

                let ret = ffi::register_relay(&self.gn, &dpid, &rust_sub, local_only, transport_type);

                self.cpp_subscriber_map.insert(key, (rust_sub, addr));
                ret
            },
            Some((rust_sub, _)) => {
                ffi::register_relay(&self.gn, &dpid, rust_sub, local_only, transport_type)
            }
        }

    }

    /// Register a relay that will act as a pass-through for the given data_product_id. It will be 
    /// a publisher and subscriber for the given data_product_id, but other components will only subscribe 
    /// to this data if they are on the same host (local_only == true), or it it is acting as a global relay (local_only == false).
    /// The Gravity infrastructure automatically handles which components should receive relayed or non-relayed data.
    /// 
    /// With parameter cache_last_value, flag used to signifgy whether or not GravityNode will cache the last setn value for a published
    /// data product. Note that using a Relay with cached_last_value = true is atypical and may result in duplicate messages received
    /// by subscribers during the relay start/stop transition.
    /// Returns success flag.
    pub fn register_relay_with_cache(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber,
                                local_only: bool, transport_type: GravityTransportType, cache_last_value: bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let key = subscriber as * const _ as usize;
        let item = self.cpp_subscriber_map.get(&key);

        match item {
            None => {
                let addr = GravityNode::get_addr(subscriber);
                let func = GravityNode::sub_filled_internal;
                let rust_sub = new_rust_subscriber(func, addr);

                let ret = ffi::register_relay_cache(&self.gn, &dpid, &rust_sub, local_only, transport_type, cache_last_value);

                self.cpp_subscriber_map.insert(key, (rust_sub, addr));
                ret
            },
            Some((rust_sub, _)) => {
                ffi::register_relay_cache(&self.gn, &dpid, rust_sub, local_only, transport_type, cache_last_value)
            }
        }
    }

    /// Unregister a relay for the given data_product_id. Handles unregistering as a publisher and subscriber.
    pub fn unregister_relay(&mut self, data_product_id: &str, subscriber: &impl GravitySubscriber) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let key = subscriber as *const _ as usize;

        let rust_sub = self.cpp_subscriber_map.get(&key);
        match rust_sub {
            None => GravityReturnCode::SUCCESS,
            Some( (sub, addr)) => {
                let temp = ffi::unregister_relay(&self.gn, &dpid, sub);
                let pointer = *addr as * mut &dyn GravitySubscriber;
                let _ = unsafe {Box::from_raw(pointer)};
                self.cpp_subscriber_map.remove(&key);
                temp
            }
        }
    }
    
    /// Returns a String representation of the provided error code.
    pub fn code_string(&self, code: GravityReturnCode) -> String {
        ffi::get_code_string(&self.gn, code).to_str().unwrap().to_string()
    }

    /// Utility method to get the host machine's IP address.
    pub fn ip(&self) -> String {
        ffi::get_IP(&self.gn).to_str().unwrap().to_string()
    }

    /// Returns the domain with which this node is associated.
    pub fn domain(&self) -> String {
        ffi::get_domain(&self.gn).to_str().unwrap().to_string()
    }
    
    fn get_addr(subscriber: &impl GravitySubscriber) -> usize {
        let boxed = Box::new(subscriber as &dyn GravitySubscriber);
        let pointer = Box::into_raw(boxed);
        pointer as usize
    }
    
    /// Creates and reuturns a FutureResponse for a delayed response to requests.
    pub fn create_future_response(&self) -> FutureResponse {
        let temp = ffi::create_future_response(&self.gn);
        FutureResponse { fr: temp }
    }

    /// Send a FutureResponse
    /// Returns success flag.
    pub fn send_future_response(&self, future_response: FutureResponse) -> GravityReturnCode {
        ffi::send_future_response(&self.gn, &future_response.fr)
    }
    
    /// Setup a GravitySubscriptionMonitor to receive subscription timeout information through
    /// the Gravity Service Directory.
    /// Returns success flag.
    pub fn set_subscription_timout_monitor(&mut self, data_product_id: &str, 
            monitor: &impl GravitySubscriptionMonitor, milli_second_timeout: i32) -> GravityReturnCode {
                self.set_subscription_timout_monitor_with_domain(data_product_id, monitor, milli_second_timeout, "", "")
            }

    /// Setup a GravitySubscriptionMonitor to receive subscription timeout information through
    /// the Gravity Service Directory.
    /// 
    /// With parameter filter.
    /// Returns success flag.            
    pub fn set_subscription_timout_monitor_with_filter(&mut self, data_product_id: &str, 
            monitor: &impl GravitySubscriptionMonitor, milli_second_timeout: i32, filter: &str) -> GravityReturnCode {
                self.set_subscription_timout_monitor_with_domain(data_product_id, monitor, milli_second_timeout, filter, "")
            }
    
    /// Setup a GravitySubscriptionMonitor to receive subscription timeout information through
    /// the Gravity Service Directory.
    /// 
    /// With parameter filter.
    /// With parameter domain.
    /// Returns success flag.  
    pub fn set_subscription_timout_monitor_with_domain(&mut self, data_product_id: &str, 
            monitor: &impl GravitySubscriptionMonitor, milli_second_timeout: i32, filter: &str, domain: &str) -> GravityReturnCode {
        
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        let key = monitor as * const _ as usize;
        let item = self.cpp_monitor_map.get(&key);

        match item {
            None => {
                let func = GravityNode::subscription_timeout_internal;
                let boxed = Box::new(monitor as &dyn GravitySubscriptionMonitor);
                let pointer = Box::into_raw(boxed);
                let addr = pointer as usize;

                let rust_monitor = ffi::new_rust_subscription_monitor(func, addr);
                let ret = ffi::set_subscription_timeout_monitor_domain(&self.gn, &dpid, &rust_monitor,  milli_second_timeout, &f, &d);


                self.cpp_monitor_map.insert(key, (rust_monitor, addr));
                ret
            },
            Some((rust_monitor, _)) => {
                ffi::set_subscription_timeout_monitor_domain(&self.gn, &dpid, rust_monitor,  milli_second_timeout, &f, &d)
            }
        }
        
    }

    /// Remove the given data_product_id from the given GravitySubscriptionMonitor.
    /// Returns success flag.
    pub fn clear_subscription_timeout_monitor(&self, data_product_id: &str, 
        monitor: &impl GravitySubscriptionMonitor) -> GravityReturnCode {
            let_cxx_string!(dpid = data_product_id);
        
        
            let key = monitor as * const _ as usize;
            let rust_monitor_op = self.cpp_monitor_map.get(&key);
            
            match rust_monitor_op {
                None => GravityReturnCode::SUCCESS,
                Some((rust_monitor, _)) => {
                    ffi::clear_subscription_timeout_monitor(&self.gn, &dpid, rust_monitor)
                }
            }   
    }

    /// Remove the given data_product_id from the given GravitySubscriptionMonitor.
    /// With parameter filter.
    /// Returns success flag.
    pub fn clear_subscription_timeout_monitor_with_filter(&self, data_product_id: &str, 
        monitor: &impl GravitySubscriptionMonitor, filter: &str) -> GravityReturnCode {
            let_cxx_string!(dpid = data_product_id);
            let_cxx_string!(f = filter);
            
            let key = monitor as * const _ as usize;
            let rust_monitor_op = self.cpp_monitor_map.get(&key);
            
            match rust_monitor_op {
                None => GravityReturnCode::SUCCESS,
                Some((rust_monitor, _)) => {
                    ffi::clear_subscription_timeout_monitor_filter(&self.gn, &dpid, rust_monitor, &f)
                }
            }
        }

    /// Remove the given data_product_id from the given GravitySubscriptionMonitor.
    /// With parameter filter.
    /// With parameter domain.
    /// Returns success flag.
    pub fn clear_subscription_timeout_monitor_with_domain(&self, data_product_id: &str, 
            monitor: &impl GravitySubscriptionMonitor, filter: &str, domain: &str) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        
        let key = monitor as * const _ as usize;
        let rust_monitor_op = self.cpp_monitor_map.get(&key);
        
        match rust_monitor_op {
            None => GravityReturnCode::SUCCESS,
            Some((rust_monitor, _)) => {
                ffi::clear_subscription_timeout_monitor_domain(&self.gn, &dpid, rust_monitor, &f, &d)
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
        let subscriber = addr as * mut &mut dyn GravitySubscriber;
        
        
        let b = unsafe {subscriber.read()};
  
        b.subscription_filled(v);
        
        
    }

    fn request_filled_internal(service_id: &CxxString, request_id: &CxxString, response: &GDataProduct, addr: usize) {
        let sid = service_id.to_str().unwrap();
        let rid = request_id.to_str().unwrap();
        let gdp = GravityNode::to_rust_gdp(response);

        let requestor = addr as * const &mut dyn GravityRequestor;
        let b = unsafe {requestor.read()};
        b.request_filled(sid, rid, &gdp);
    }

    fn request_timeout_internal(service_id: &CxxString, request_id: &CxxString, addr: usize) {
        let sid = service_id.to_str().unwrap();
        let rid = request_id.to_str().unwrap();

        let requestor = addr as * const &mut dyn GravityRequestor;
        let b = unsafe {requestor.read()};
        b.request_timeout(sid, rid);
    }

    fn request_internal(service_id: &CxxString, data_product: &GDataProduct, addr: usize) -> SharedPtr<GDataProduct>{
        let gdp = GravityNode::to_rust_gdp(data_product);
        let sid = service_id.to_str().unwrap();

        let p = addr as * mut &mut dyn GravityServiceProvider;
        let service_provider = unsafe {p.read()};
        let g = service_provider.request(sid, &gdp);
        ffi::copy_gdp_shared(&g.gdp)
    }
    
    fn missed_heartbeat_internal(component_id: &CxxString, microsecond_to_last_heartbeat: i64,
        interval_in_microseconds: &mut i64, addr: usize){

        let cid = component_id.to_str().unwrap();
        let p = addr as * mut &mut dyn GravityHeartbeatListener;
        let listener = unsafe {p.read()};
        listener.missed_heartbeat(cid, microsecond_to_last_heartbeat, interval_in_microseconds);
    }

    fn received_heartbeat_internal(component_id: &CxxString, interval_in_microseconds: &mut i64, addr: usize) {
        let cid = component_id.to_str().unwrap();
        let p = addr as * mut &mut dyn GravityHeartbeatListener;
        let listener = unsafe { p.read() };

        listener.received_heartbeat(cid, interval_in_microseconds);
    }
    fn subscription_timeout_internal (data_product_id: &CxxString, milli_seconds_since_last: i32,
            filter: &CxxString, domain: &CxxString, addr: usize) {
        let dpid = data_product_id.to_str().unwrap();
        let f = filter.to_str().unwrap();
        let d = domain.to_str().unwrap();

        let p = addr as * mut &mut dyn GravitySubscriptionMonitor;
        let monitor = unsafe { p.read() };

        monitor.subscription_timeout(dpid, milli_seconds_since_last, f, d);
    }
    
}

impl Drop for GravityNode {
    fn drop(&mut self) {

        for (_ , (_, item)) in self.cpp_requestor_provider_map.iter() {
            let pointer = *item as * mut &dyn GravityRequestor;
            let _ = unsafe {Box::from_raw(pointer)};
        }
        for (_ , (_, item)) in self.cpp_subscriber_map.iter() {
            let pointer = *item as * mut &dyn GravitySubscriber;
            let _= unsafe {Box::from_raw(pointer)};
        }
        for (_ , (_, item)) in self.cpp_service_provider_map.iter() {
            let pointer = *item as * mut &dyn GravityServiceProvider;
            let _= unsafe {Box::from_raw(pointer)};
        }
        for (_ , (_, item)) in self.cpp_monitor_map.iter() {
            let pointer = *item as * mut &dyn GravitySubscriptionMonitor;
            let _= unsafe {Box::from_raw(pointer)};
        }
        for (_ , (_, item,id, domain)) in self.cpp_listener_map.iter() {
            self.unregister_heartbeat_listener_with_domain(id, domain);
            let pointer = *item as * mut &dyn GravityHeartbeatListener;
            let _= unsafe {Box::from_raw(pointer)};
        }
        // std::thread::sleep(time::Duration::from_secs(2));

    }
}