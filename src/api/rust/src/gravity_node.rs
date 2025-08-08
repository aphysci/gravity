#![allow(dead_code)]
use std::collections::HashMap;
use std::sync::atomic::AtomicI32;
use std::sync::Arc;
use cxx::{let_cxx_string, CxxString, CxxVector, SharedPtr, UniquePtr};
use crate::ffi::*;
use crate::gravity_heartbeat_listener::{GravityHeartbeatListener, ListenerWrap};
use crate::gravity_subscription_monitor::{GravitySubscriptionMonitor, MonitorWrap};
use crate::{ffi, gravity_subscriber::*,
     gravity_data_product::*, gravity_requestor::*, gravity_service_provider::*
    , future_response::*};

/// Return codes used on a Gravity system.
pub type GravityReturnCode = GravityReturnCodes;

/// Network transport protocols available on a Gravity system.
pub type GravityTransportType = GravityTransportTypes;

static COUNTER: AtomicI32 = AtomicI32::new(0);

/// A component that provides a simple interface point to a Gravity-enabled application.
pub struct GravityNode {
    gn: UniquePtr<GNode>,
    subscribers_map: HashMap<i32, (Arc<SubscriberWrap>, UniquePtr<RustSubscriber>)>,
    service_provider_map: HashMap<String, (Arc<ServiceWrap>, UniquePtr<RustServiceProvider>)>,
    requestor_provider_map: HashMap<i32, (Arc<RequestorWrap>, UniquePtr<RustRequestor>)>,
    listener_map: HashMap<(String, String), (Arc<ListenerWrap>, UniquePtr<RustHeartbeatListener>)>,
    monitor_map: HashMap<i32, (Arc<MonitorWrap>, UniquePtr<RustSubscriptionMonitor>)>,
}

impl GravityNode {
    /// Default constructor.
    pub fn new() -> GravityNode {
        GravityNode { 
            gn: ffi::GravityNode(), 
            subscribers_map: HashMap::new(),
            service_provider_map: HashMap::new(),
            requestor_provider_map: HashMap::new(),
            listener_map: HashMap::new(),
            monitor_map: HashMap::new(),
        }
    }
    
    /// Constructor that also initializes.
    /// Uses component_id to initialize.
    pub fn with_id(component_id: &str) -> GravityNode {
        let_cxx_string!(cid = component_id);
        GravityNode { 
            gn: ffi::gravity_node_id(&cid), 
            subscribers_map: HashMap::new(),
            service_provider_map: HashMap::new(),
            requestor_provider_map: HashMap::new(),
            listener_map: HashMap::new(),
            monitor_map: HashMap::new(),
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

    /// Gets a token for use in subscribing, unsubscribing, and registering and unregistering relays.
    /// Dropping a token does not modify the GravityNode (including subscriptions)
    /// Dropping a token will not cause a subscription to cancel, only unsubscribing and dropping the GravityNode will
    /// Once a token is dropped, it and it's information cannot be recovered, so make sure to keep it in scope if you would
    /// like to subscribe or unsubscribe with it.
    pub fn tokenize_subscriber(&mut self, subscriber: impl GravitySubscriber + 'static) -> SubscriberToken {
        let boxed = Box::new(subscriber) as Box<dyn GravitySubscriber>;
        let mut wrap = Arc::new(SubscriberWrap { subscriber: boxed });
        let cpp_sub = unsafe {
            ffi::new_rust_subscriber(GravityNode::sub_filled_internal,
                 Arc::get_mut(&mut wrap).unwrap() as * mut SubscriberWrap)
        };
        let key = COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        let tok = SubscriberToken { key: key };
        self.subscribers_map.insert(key, (wrap, cpp_sub));
        tok
    }
    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriber is a token that has been tokenized with the GravityNode.
    /// If the token has not been registered with this GravityNode, returns NOT_REGISTERED return code
    pub fn subscribe(&mut self, data_product_id: &str, subscriber: &SubscriberToken) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);

        let item = self.subscribers_map.get(&subscriber.key);
        match item {
            Some( (_, cpp_sub)) => ffi::subscribe(&self.gn, &dpid, cpp_sub),
            None => GravityReturnCodes::NOT_REGISTERED,
        } 
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriber is a token that has been tokenized with the GravityNode.
    /// Adds optional parameter filter, a text filter to apply to subscription.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn subscribe_with_filter(&mut self, data_product_id: &str, subscriber: &SubscriberToken, filter: &str) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        
        let item = self.subscribers_map.get(&subscriber.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some( (_, cpp_sub)) => {
                ffi::subscribe_filter(&self.gn, &dpid, cpp_sub, &f)
            }
        }
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriber is a token that has been tokenized with the GravityNode.
    /// Adds optional parameter filter, a text filter to apply to subscription.
    /// Adds optional parameter domain, the domain of the network components.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn subscribe_with_domain(&mut self, data_product_id: &str, subscriber: SubscriberToken, filter: &str, domain: &str) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        
        let item = self.subscribers_map.get(&subscriber.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some( (_, cpp_sub)) => {
                ffi::subscribe_domain(&self.gn, &dpid, cpp_sub, &f, &d)
            }
        }
    }

    /// Setup a subscription to a data product throguh the Gravity Service Directory.
    /// The data_product_id param is the ID of the data product of interest.
    /// The Subscriber is a token that has been tokenized with the GravityNode.
    /// Adds optional parameter filter, a text filter to apply to subscription.
    /// Adds optional parameter domain, the domain of the network components.
    /// Adds optional paramter receive_last_cache_value.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn subscribe_with_cache(&mut self, data_product_id: &str, subscriber: &SubscriberToken, filter: &str, domain: &str, recieve_last_cache_value: bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        
        let item = self.subscribers_map.get(&subscriber.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some( (_, cpp_sub)) => {
                ffi::subscribe_cache(&self.gn, &dpid, cpp_sub, &f, &d, recieve_last_cache_value)
            }
        }
    }
    
    /// Un-subscribe from a data product.
    pub fn unsubscribe(&mut self, data_product_id: &str, subscriber: &SubscriberToken) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let item  = self.subscribers_map.get(&subscriber.key);
        match item {
            Some( (_, cpp_sub)) => ffi::unsubscribe(&self.gn, &dpid, cpp_sub),
            None => GravityReturnCodes::SUCCESS,
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

    /// Get a token to use to request services asynchronously
    /// Dropping a token does not change the GravityNode in any way.
    /// Once a token is dropped, the information it and the information it contains is not recoverable
    pub fn tokenize_requestor (&mut self, requestor: impl GravityRequestor + 'static) -> RequestorToken{
        let boxed = Box::new(requestor) as Box<dyn GravityRequestor>;
        let mut wrap = Arc::new(RequestorWrap { requestor: boxed });
        let cpp_sub = unsafe {
            ffi::new_rust_requestor(
                GravityNode::request_filled_internal,
                GravityNode::request_timeout_internal,
                Arc::get_mut(&mut wrap).unwrap() as * mut RequestorWrap)
        };
        let key = COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        let tok = RequestorToken { key: key };
        self.requestor_provider_map.insert(key, (wrap, cpp_sub));
        tok

    }
    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor is a token that has been tokenizedd with the GravityNode.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn request_async(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &RequestorToken) -> GravityReturnCode {
        
        let_cxx_string!(sid = service_id);
        
        let item = self.requestor_provider_map.get(&requestor.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some((_, cpp_req)) => {
                ffi::request_async(&self.gn, &sid, &request.gdp, cpp_req)
            }
        }
       
    }
    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor is a token that has been tokenize with the GravityNode.
    /// With parameter request_id, the identifier for this request.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn request_async_with_request_id(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &RequestorToken, request_id: &str) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        let_cxx_string!(req = request_id);

        let item = self.requestor_provider_map.get(&requestor.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some((_, cpp_req)) => {
                ffi::request_async_request_id(&self.gn, &sid, &request.gdp, cpp_req, &req)
            }
        }
    }

    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor is a token that has been tokenize with the GravityNode.
    /// With parameter request_id, the identifier for this request.
    /// With parameter timeout_milliseconds, the timeout in Milliseconds (-1 for no timeout).
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn request_async_with_timeout(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &RequestorToken, request_id: &str, timeout_milliseconds: i32) -> GravityReturnCode {
            
        let_cxx_string!(sid = service_id);
        let_cxx_string!(req = request_id);

        let item = self.requestor_provider_map.get(&requestor.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some((_, cpp_req)) => {
                ffi::request_async_timeout(&self.gn, &sid, &request.gdp, cpp_req, &req, timeout_milliseconds)
            }
        }
    }

    /// Make an asynchronous request against a service provider through the Gravity Service Directory.
    /// requestor is a token that has been tokenize with the GravityNode.
    /// With parameter request_id, the identifier for this request.
    /// With parameter timeout_milliseconds, the timeout in Milliseconds (-1 for no timeout).
    /// With parameter domain, the domain of the network components.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn request_async_with_domain(&mut self, service_id:  &str, request: &GravityDataProduct, 
        requestor: &RequestorToken, request_id: &str, timeout_milliseconds: i32, domain: &str) -> GravityReturnCode
    {
        let_cxx_string!(sid = service_id);
        let_cxx_string!(req = request_id);
        let_cxx_string!(dom = domain);

        let item = self.requestor_provider_map.get(&requestor.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some((_, cpp_req)) => {
                ffi::request_async_domain(&self.gn, &sid, &request.gdp, cpp_req, &req, timeout_milliseconds, &dom)
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

    /// Get a token to register services
    /// Gives GravityNode ownership of the GravityServiceProvider so it controls its scope
    /// Dropping a token will not affect the GravityNode in any way (including its registered services)
    /// Use caution since once a token its dropped, it is not recoverable.
    // pub fn tokenize_service(&mut self, server: impl GravityServiceProvider + 'static) -> ServiceToken {
    //     let boxed = Box::new(server) as Box<dyn GravityServiceProvider>;
    //     let mut wrap = Arc::new(ServiceWrap { service: boxed });
    //     let cpp_sub = unsafe {
    //         ffi::new_rust_service_provider(
    //             GravityNode::request_internal,
    //              Arc::get_mut(&mut wrap).unwrap() as * mut ServiceWrap)
    //     };
    //     let key = COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
    //     let tok = ServiceToken { key: key };
    //     self.service_provider_map.insert(key, (wrap, cpp_sub));
    //     tok
    // }

    /// Registers as a service provider with Gravity.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn register_service(&mut self, service_id:  &str, transport_type: GravityTransportType, server: impl GravityServiceProvider + 'static) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        
        let boxed = Box::new(server) as Box<dyn GravityServiceProvider>;
        let mut wrap = Arc::new(ServiceWrap { service: boxed });
        let cpp_sub = unsafe {
            ffi::new_rust_service_provider(
                GravityNode::request_internal,
                 Arc::get_mut(&mut wrap).unwrap() as * mut ServiceWrap)
        };
        let key = service_id.to_string();
        let exists = self.service_provider_map.insert(key.clone(), (wrap, cpp_sub));
        
        match exists {
            Some( _ )  => {ffi::unregister_service(&self.gn, &sid);},
            None => (),
        }
        

        let (_, cpp_sub) = self.service_provider_map.get(&key).unwrap();
        ffi::register_service(&self.gn, &sid, transport_type, cpp_sub)
    }

    // Unregister as a service provider with the Gravity Service Directory.
    // Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn unregister_service(&self, service_id: &str) -> GravityReturnCode {
        let_cxx_string!(sid = service_id);
        ffi::unregister_service(&self.gn, &sid)
    }


    /// Gets a token to use to register a heartbeat listener corresponding to the GravityHeartbeatListener provided.
    /// Dropping a token does not affect the GravityNode and it and its information are irrecoverable
    // pub fn tokenize_heartbeat_listener(&mut self, listener: impl GravityHeartbeatListener + 'static) -> HeartbeatListenerToken {
    //     let boxed = Box::new(listener);

    //     let mut wrap = Arc::new(ListenerWrap { listener: boxed });
    //     let cpp_listener = unsafe {
    //         ffi::new_rust_heartbeat_listener(
    //             GravityNode::missed_heartbeat_internal,
    //             GravityNode::received_heartbeat_internal,
    //             Arc::get_mut(&mut wrap).unwrap() as * mut ListenerWrap)
    //     };
    //     let key = COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
    //     let tok = HeartbeatListenerToken { key: key };
    //     self.listener_map.insert(key, (wrap, cpp_listener));
    //     tok
    // }
    /// Registers a callback to be called when we don't get a heartbeat from another component.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn register_heartbeat_listener(&mut self, component_id: &str, interval_in_microseconds: i64, listener: impl GravityHeartbeatListener + 'static) -> GravityReturnCode{
        let_cxx_string!(cid = component_id);

        let boxed = Box::new(listener);

        let mut wrap = Arc::new(ListenerWrap { listener: boxed });
        let cpp_listener = unsafe {
            ffi::new_rust_heartbeat_listener(
                GravityNode::missed_heartbeat_internal,
                GravityNode::received_heartbeat_internal,
                Arc::get_mut(&mut wrap).unwrap() as * mut ListenerWrap)
        };
            
        let ret = ffi::register_heartbeat_listener(
            &self.gn,
            &cid, 
            interval_in_microseconds, 
            &cpp_listener);
           
        self.listener_map.insert((component_id.to_string(), "".to_string()), (wrap, cpp_listener));
        ret

    }

    /// Registers a callback to be called when we don't get a heartbeat from another component.
    /// With paramter domain, the name of the domain for the component_id.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn register_heartbeat_listener_with_domain(&mut self, component_id: &str, interval_in_microseconds: i64,
                                        listener: impl GravityHeartbeatListener + 'static, domain: &str) -> GravityReturnCode
    {
        let_cxx_string!(cid = component_id);
        let_cxx_string!(d = domain);

        let boxed = Box::new(listener);

        let mut wrap = Arc::new(ListenerWrap { listener: boxed });
        let cpp_listener = unsafe {
            ffi::new_rust_heartbeat_listener(
                GravityNode::missed_heartbeat_internal,
                GravityNode::received_heartbeat_internal,
                Arc::get_mut(&mut wrap).unwrap() as * mut ListenerWrap)
        };
            
        let ret = ffi::register_heartbeat_listener_domain(
            &self.gn,
            &cid, 
            interval_in_microseconds, 
            &cpp_listener,
            &d);
           
        self.listener_map.insert((component_id.to_string(), domain.to_string()), (wrap, cpp_listener));
        ret
         
    }

    /// Unregisters a callback for when we get a heartbeat from another component.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn unregister_heartbeat_listener(&mut self, component_id: &str) -> GravityReturnCode {
        let_cxx_string!(cid = component_id);
        let ret = ffi::unregister_heartbeat_listener(&self.gn, &cid);
        self.listener_map.remove(&(component_id.to_string(), "".to_string()));
        ret
    }

    /// Unregisters a callback for when we get a heartbeat from another component.
    /// With paramter domain, the name of the domain for the component_id.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn unregister_heartbeat_listener_with_domain(&mut self, component_id: &str, domain: &str) -> GravityReturnCode {
        let_cxx_string!(cid = component_id);
        let_cxx_string!(d = domain);
        let ret = ffi::unregister_heartbeat_listener_domain(&self.gn, &cid, &d);
        self.listener_map.remove(&(component_id.to_string(), domain.to_string()));
        ret
    }

    /// Register a relay that will act as a pass-through for the given data_product_id. It will be 
    /// a publisher and subscriber for the given data_product_id, but other components will only subscribe 
    /// to this data if they are on the same host (local__only == true), or it it is acting as a global relay (local_only == false).
    /// The Gravity infrastructure automatically handles which components should receive relayed or non-relayed data.
    /// 
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn register_relay(&mut self, data_product_id: &str, subscriber: &SubscriberToken,
                          local_only: bool, transport_type: GravityTransportType) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        
        let item = self.subscribers_map.get(&subscriber.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some( (_, cpp_sub) ) => {
                ffi::register_relay(
                    &self.gn,
                    &dpid,
                    cpp_sub,
                    local_only,
                    transport_type)
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
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode
    pub fn register_relay_with_cache(&mut self, data_product_id: &str, subscriber: &SubscriberToken,
                                local_only: bool, transport_type: GravityTransportType, cache_last_value: bool) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        
        let item = self.subscribers_map.get(&subscriber.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some( (_, cpp_sub) ) => {
                ffi::register_relay_cache(
                    &self.gn,
                    &dpid,
                    cpp_sub,
                    local_only,
                    transport_type,
                    cache_last_value)
            }
        }
    }

    /// Unregister a relay for the given data_product_id. Handles unregistering as a publisher and subscriber.
    pub fn unregister_relay(&mut self, data_product_id: &str, subscriber: &SubscriberToken) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        
        let item = self.subscribers_map.get(&subscriber.key);
        
        match item {
            None => GravityReturnCodes::SUCCESS,
            Some( (_,cpp_sub) ) => {
                ffi::unregister_relay(&self.gn, &dpid, cpp_sub)
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
    
    /// Use a GravitySubscriptionMonitor to get a token
    /// Stores the trait object within the GravityNode so the GravityNode has ownership
    /// Use the token for setting and clearing subscription monitor.
    /// Droppping the token will NOT have any effect on the GravityNode, so use caution when tokens go out of scope. 
    /// They are not recoverable
    pub fn tokenize_subscription_monitor(&mut self, monitor: impl GravitySubscriptionMonitor + 'static) -> SubscriptionMonitorToken {

        let boxed = Box::new(monitor) as Box<dyn GravitySubscriptionMonitor>;
        let mut wrap = Arc::new(MonitorWrap { monitor: boxed });

        let cpp_monitor = unsafe {
            ffi::new_rust_subscription_monitor(GravityNode::subscription_timeout_internal, Arc::get_mut(&mut wrap).unwrap() as * mut MonitorWrap)
        }; 

        let key = COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        let tok = SubscriptionMonitorToken { key: key };
        self.monitor_map.insert(key, (wrap, cpp_monitor));
        tok
    }
    /// Setup a GravitySubscriptionMonitor to receive subscription timeout information through
    /// the Gravity Service Directory.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn set_subscription_timout_monitor(&mut self, data_product_id: &str, 
            monitor: &SubscriptionMonitorToken, milli_second_timeout: i32) -> GravityReturnCode {
                
            let_cxx_string!(dpid = data_product_id);
            
            let item = self.monitor_map.get(&monitor.key);
            match item {
                None => GravityReturnCodes::NOT_REGISTERED,
                Some( (_, cpp_monitor)) => {
                    ffi::set_subscription_timeout_monitor(
                        &self.gn,
                        &dpid,
                        cpp_monitor,
                        milli_second_timeout)
                }
            }
    }

    /// Setup a GravitySubscriptionMonitor to receive subscription timeout information through
    /// the Gravity Service Directory.
    /// 
    /// With parameter filter.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.     
    pub fn set_subscription_timout_monitor_with_filter(&mut self, data_product_id: &str, 
            monitor: &SubscriptionMonitorToken, milli_second_timeout: i32, filter: &str) -> GravityReturnCode {
                
                let_cxx_string!(dpid = data_product_id);
                let_cxx_string!(f = filter);

                let item = self.monitor_map.get(&monitor.key);

                match item {
                    None => GravityReturnCodes::NOT_REGISTERED,
                    Some( (_, cpp_monitor)) => {
                        ffi::set_subscription_timeout_monitor_filter(
                            &self.gn,
                            &dpid,
                            cpp_monitor,
                            milli_second_timeout,
                            &f)
                    }

                }
            
            }
    
    /// Setup a GravitySubscriptionMonitor to receive subscription timeout information through
    /// the Gravity Service Directory.
    /// 
    /// With parameter filter.
    /// With parameter domain.
    /// Returns success flag or not_registered flag if the token is not tokenized with the current GravityNode.
    pub fn set_subscription_timout_monitor_with_domain(&mut self, data_product_id: &str, 
            monitor: &SubscriptionMonitorToken, milli_second_timeout: i32, filter: &str, domain: &str) -> GravityReturnCode {
        
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        
        let item = self.monitor_map.get(&monitor.key);

        match item {
            None => GravityReturnCodes::NOT_REGISTERED,
            Some( (_, cpp_monitor)) => {
                ffi::set_subscription_timeout_monitor_domain(
                    &self.gn,
                    &dpid,
                    cpp_monitor,
                    milli_second_timeout,
                    &f,
                &d)
            }
            
        }
        
    }

    /// Remove the given data_product_id from the given GravitySubscriptionMonitor.
    /// Returns success flag.
    pub fn clear_subscription_timeout_monitor(&self, data_product_id: &str, 
        monitor: &SubscriptionMonitorToken) -> GravityReturnCode {
            let_cxx_string!(dpid = data_product_id);
        
            let item = self.monitor_map.get(&monitor.key);
            
            match item {
                None => GravityReturnCode::SUCCESS,
                Some((_, cpp_monitor)) => {
                    ffi::clear_subscription_timeout_monitor(
                        &self.gn,
                        &dpid,
                        cpp_monitor)
                }
            }   
    }

    /// Remove the given data_product_id from the given GravitySubscriptionMonitor.
    /// With parameter filter.
    /// Returns success flag.
    pub fn clear_subscription_timeout_monitor_with_filter(&self, data_product_id: &str, 
        monitor: &SubscriptionMonitorToken, filter: &str) -> GravityReturnCode {
            let_cxx_string!(dpid = data_product_id);
            let_cxx_string!(f = filter);
            
           
            let item = self.monitor_map.get(&monitor.key);
            
            match item {
                None => GravityReturnCode::SUCCESS,
                Some((_, cpp_monitor)) => {
                    ffi::clear_subscription_timeout_monitor_filter(
                        &self.gn,
                        &dpid,
                        cpp_monitor,
                        &f)
                }
            }
        }

    /// Remove the given data_product_id from the given GravitySubscriptionMonitor.
    /// With parameter filter.
    /// With parameter domain.
    /// Returns success flag.
    pub fn clear_subscription_timeout_monitor_with_domain(&self, data_product_id: &str, 
            monitor: &SubscriptionMonitorToken, filter: &str, domain: &str) -> GravityReturnCode {
        let_cxx_string!(dpid = data_product_id);
        let_cxx_string!(f = filter);
        let_cxx_string!(d = domain);
        
        let item = self.monitor_map.get(&monitor.key);
        
        match item {
            None => GravityReturnCode::SUCCESS,
            Some((_, cpp_monitor)) => {
                ffi::clear_subscription_timeout_monitor_domain(
                    &self.gn,
                    &dpid,
                    cpp_monitor,
                    &f,
                    &d)
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

    fn sub_filled_internal(data_products: &CxxVector<GDataProduct>, ptr: * mut SubscriberWrap) {
       
        let v = GravityNode::rustify(data_products);
        let next = unsafe {
            &mut (*ptr)
        };
        let subscriber = &mut next.subscriber;
        subscriber.subscription_filled(v);
        
        
        
    }

    fn request_filled_internal(service_id: &CxxString, request_id: &CxxString, response: &GDataProduct, ptr: * mut RequestorWrap) {
        let sid = service_id.to_str().unwrap();
        let rid = request_id.to_str().unwrap();
        let gdp = GravityNode::to_rust_gdp(response);

        let next = unsafe {
            &mut (*ptr)
        };
        let requestor = &mut next.requestor;
        requestor.request_filled(sid, rid, &gdp);
    }

    fn request_timeout_internal(service_id: &CxxString, request_id: &CxxString, ptr: * mut RequestorWrap) {
        let sid = service_id.to_str().unwrap();
        let rid = request_id.to_str().unwrap();

        let next = unsafe {
            &mut (*ptr)
        };
        let requestor = &mut next.requestor;
        requestor.request_timeout(sid, rid);
    }

    fn request_internal(service_id: &CxxString, data_product: &GDataProduct, ptr: * mut ServiceWrap) -> SharedPtr<GDataProduct>{
        let gdp = GravityNode::to_rust_gdp(data_product);
        let sid = service_id.to_str().unwrap();

        let next = unsafe {
            &mut (*ptr)
        };
        let service_provider = &mut next.service;
        let g = service_provider.request(sid, &gdp);
        ffi::copy_gdp_shared(&g.gdp)
    }
    
    fn missed_heartbeat_internal(component_id: &CxxString, microsecond_to_last_heartbeat: i64,
        interval_in_microseconds: &mut i64, ptr: * mut ListenerWrap){

        let cid = component_id.to_str().unwrap();
        let next = unsafe {
            &mut (*ptr)
        };
        let listener = &mut next.listener;
        listener.missed_heartbeat(cid, microsecond_to_last_heartbeat, interval_in_microseconds);
    }

    fn received_heartbeat_internal(component_id: &CxxString, interval_in_microseconds: &mut i64, ptr: * mut ListenerWrap) {
        let cid = component_id.to_str().unwrap();
        
        let next = unsafe {
            &mut (*ptr)
        };
        let listener = &mut next.listener;
        listener.received_heartbeat(cid, interval_in_microseconds);
    }

    fn subscription_timeout_internal (data_product_id: &CxxString, milli_seconds_since_last: i32,
            filter: &CxxString, domain: &CxxString, ptr: * mut MonitorWrap) {
        let dpid = data_product_id.to_str().unwrap();
        let f = filter.to_str().unwrap();
        let d = domain.to_str().unwrap();

        let next = unsafe {
            &mut (*ptr)
        };
        let monitor = &mut next.monitor;
        monitor.subscription_timeout(dpid, milli_seconds_since_last, f, d);
    }
    
}

/// Token used to subscribe and unsubscribe. As well as register and unregister relays.
/// Must use the token on the GravityNode that you tokenized with.
pub struct SubscriberToken {
    pub(crate) key: i32,
}

/// Token used to request against a service provider
/// Must use the token on the GravityNode that you tokenized with.
pub struct RequestorToken {
    pub(crate) key: i32,
}

/// Token to register as a service provider the GravityServiceProvider used to create this token.
/// Must use the token on the GravityNode that you tokenized with.
pub struct ServiceToken {
    pub(crate) key: i32,
}

/// Token to register as a Heartbeat listener.
/// Must use the token on the GravityNode that you tokenized with.
pub struct HeartbeatListenerToken {
    pub(crate) key: i32,
}

/// Token used to set and clear a subscription monitor
/// Must use the token on the GravityNode that you tokenized with.
pub struct SubscriptionMonitorToken {
    pub(crate) key: i32,
}