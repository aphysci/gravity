#![allow(non_camel_case_types)]
#![allow(dead_code)]
pub use ffi::*;

#[cxx::bridge]
mod ffi {

   
    #[namespace = "gravity"]
    #[repr(i32)]
    #[derive(Debug)]
    #[cxx_name = "GravityReturnCode"]
    pub enum GReturnCode {
        SUCCESS = 0,                ///< The request was successful
        FAILURE = -1,               ///< The request failed
        NO_SERVICE_DIRECTORY = -2,  ///< Could not find Service Directory
        REQUEST_TIMEOUT = -3,       ///< The request timed out while waiting for a response.
        DUPLICATE = -4,             ///< Attempting to register an existing data product ID
        REGISTRATION_CONFLICT =
            -5,  ///< Attempting to unregister from a service or dataProductID that is not currently registered.
        NOT_REGISTERED = -6,  ///< TBD
        NO_SUCH_SERVICE =
            -7,  ///< Made a bad request (Ex: used a non-existent domain or submitted a request before service was available)
        LINK_ERROR = -8,            ///< Error serializing or deserializing a Gravity Data Product
        INTERRUPTED = -9,           ///< Received an interruption.
        NO_SERVICE_PROVIDER = -10,  ///< No service provider found.
        NO_PORTS_AVAILABLE = -11,   ///< No ports available
        INVALID_PARAMETER = -12,    ///< Invalid parameter. (Ex: entered a negative number for time)
        NOT_INITIALIZED =
            -13,  ///< The GravityNode has not successfully completed initialization yet (i.e. init has not been called or did not succeed).
        RESERVED_DATA_PRODUCT_ID = -14, 
    }
    #[namespace = "gravity"]
    #[repr(i32)]
    #[cxx_name = "GravityTransportType"]
    pub enum GTransportType {
        TCP = 0,     ///< Transmission Control Protocol
        INPROC = 1,  ///< In-process (Inter-thread) Communication
        PGM = 2,     ///< Pragmatic General Multicast Protocol
        EPGM = 3,    ///< Encapsulated PGM
        IPC = 4,  //< Inter-Process Communication
    }
    #[namespace = "gravity"]
    unsafe extern "C++" {
        include!("gravity/lib/RustGravityNode.h");
        include!("gravity/lib/RustGravityDataProduct.h");
        include!("gravity/lib/RustFutureResponse.h");
        include!("gravity/lib/RustGravitySubscriber.h");
        include!("gravity/lib/RustGravityHeartbeatListener.h");
        include!("gravity/lib/RustGravitySubscriptionMonitor.h");
        include!("gravity/lib/RustGravityServiceProvider.h");



        #[rust_name = "GReturnCode"]
        type GravityReturnCode;

        #[rust_name = "GTransportType"]
        type GravityTransportType;
        #[rust_name = "GNode"]
        type GravityNode; 

        #[rust_name = "GDataProduct"]
        type GravityDataProduct;
 
        #[rust_name = "GFutureResponse"]
        type FutureResponse; 
        type RustSubscriber;
        type RustRequestor;
        type RustServiceProvider;
        type RustHeartbeatListener;
        type RustSubscriptionMonitor;
        
        //GravityNode methods
        #[rust_name = "GravityNode"]
        fn newGravityNode() -> UniquePtr<GNode>;

        #[rust_name = "gravity_node_id"]
        fn newGravityNodeId(componentID: &CxxString) -> UniquePtr<GNode>;

        #[rust_name = "init"]
        fn rustInit(gn: &UniquePtr<GNode>, componentID: &CxxString) -> GReturnCode;
        
        #[rust_name = "init_default"]
        fn rustInit(gn: &UniquePtr<GNode>) -> GReturnCode;

        #[rust_name = "wait_for_exit"]
        fn rustWaitForExit(gn: &UniquePtr<GNode>);

        #[rust_name = "publish"]
        fn rustPublish(gn: &UniquePtr<GNode>, dataProduct: &UniquePtr<GDataProduct>) -> GReturnCode;

        #[rust_name = "subsribers_exist"]
        fn rustSubscribersExist(gn: &UniquePtr<GNode>, dataProductID: &CxxString, has_subscribers: &mut bool) -> GReturnCode;

        #[rust_name = "start_heartbeat"]
        fn rustStartHeartbeat(gn: &UniquePtr<GNode>, interval_in_microseconds: i64) -> GReturnCode;

        #[rust_name = "stop_heartbeat"]
        fn rustStopHeartbeat(gn: &UniquePtr<GNode>) -> GReturnCode;

        #[rust_name = "get_string_param"]
        fn rustGetStringParam(gn: &UniquePtr<GNode>, key: &CxxString, default_value: &CxxString) -> UniquePtr<CxxString>;
        
        #[rust_name = "get_int_param"]
        fn rustGetIntParam(gn: &UniquePtr<GNode>, key: &CxxString, default_value: i32) -> i32;
        
        #[rust_name = "get_float_param"]
        fn rustGetFloatParam(gn: &UniquePtr<GNode>, key: &CxxString, default_value: f64) -> f64;

        #[rust_name = "get_bool_param"]
        fn rustGetBoolParam(gn: &UniquePtr<GNode>, key: &CxxString, default_value: bool) -> bool;

        #[rust_name = "get_component_ID"]
        fn rustGetComponentID(gn: &UniquePtr<GNode>) -> UniquePtr<CxxString>;
        

        #[rust_name = "register_data_product"]
        fn rustRegisterDataProduct(gn: &UniquePtr<GNode>, dataProductID: &CxxString, 
            transportType: GTransportType) -> GReturnCode;

        #[rust_name = "register_data_product_cache"]
        fn rustRegisterDataProduct(gn: &UniquePtr<GNode>, dataProductID: &CxxString, 
            transportType: GTransportType, cacheLastValue: bool) -> GReturnCode;

        #[rust_name = "unregister_data_product"]
        fn rustUnregisterDataProduct(gn: &UniquePtr<GNode>, dataProductID: &CxxString) -> GReturnCode;

        #[rust_name = "get_code_string"]
        fn rustGetCodeString(gn: &UniquePtr<GNode>, code: GReturnCode) -> UniquePtr<CxxString>;
        
        #[rust_name = "get_IP"]
        fn rustGetIP(gn: &UniquePtr<GNode>) -> UniquePtr<CxxString>;

        #[rust_name = "get_domain"]
        fn rustGetDomain(gn: &UniquePtr<GNode>) -> UniquePtr<CxxString>;

        #[rust_name = "subscribe"]
        fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString,  subscriber: &UniquePtr<RustSubscriber>) -> GReturnCode;
        
        #[rust_name = "subscribe_filter"]
        fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString,  subscriber: &UniquePtr<RustSubscriber>,
             filter: &CxxString) -> GReturnCode;

        #[rust_name = "subscribe_domain"]
        fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString,  subscriber: &UniquePtr<RustSubscriber>,
             filter: &CxxString, domain: &CxxString) -> GReturnCode;

        #[rust_name = "subscribe_cache"]
        fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString,  subscriber: &UniquePtr<RustSubscriber>,
             filter: &CxxString, domain: &CxxString, recieve_last_cached_value: bool) -> GReturnCode;

        #[rust_name = "unsubscribe"]
        fn rustUnsubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString, subscriber: &UniquePtr<RustSubscriber>) -> GReturnCode;
        
        #[rust_name = "new_rust_subscriber"]
        fn newRustSubscriber(func: fn(&CxxVector<GDataProduct>, usize), addr: usize) -> UniquePtr<RustSubscriber>;

        #[rust_name = "new_rust_requestor"]
        fn rustRustRequestor(filled: fn(&CxxString, &CxxString, &GDataProduct, usize), timeout: fn(&CxxString, &CxxString, usize), addr: usize) -> UniquePtr<RustRequestor>;
        
        #[rust_name = "new_rust_heartbeat_listener"]
        fn rustNewHeartbeatListener(missed: fn(&CxxString, i64, &mut i64, usize), received: fn(&CxxString, &mut i64, usize), addr: usize) -> UniquePtr<RustHeartbeatListener>;
        
        #[rust_name = "new_rust_subscription_monitor"]
        fn rustNewSubscriptionMonitor(func: fn(&CxxString, i32, &CxxString, &CxxString, usize), addr: usize) -> UniquePtr<RustSubscriptionMonitor>;

        #[rust_name = "register_heartbeat_listener"]
        fn rustRegisterHeartbeatListener(gn: &UniquePtr<GNode>, component_id: &CxxString, interval_in_microseconds: i64, listener: &UniquePtr<RustHeartbeatListener>, domain: &CxxString) -> GReturnCode;
        
        #[rust_name = "unregister_heartbeat_listener"]
        fn rustUnregisterHeartbeatListener(gn: &UniquePtr<GNode>, component_id: &CxxString, domain: &CxxString) -> GReturnCode;
        
        #[rust_name = "new_rust_service_provider"]
        fn rustRustServiceProvider(func: fn(&CxxString, &GDataProduct, usize) -> SharedPtr<GDataProduct>, addr: usize) -> UniquePtr<RustServiceProvider>;

        #[rust_name = "request_async"]
        fn rustRequest(gn: &UniquePtr<GNode>, service_id: &CxxString, dataProduct: &UniquePtr<GDataProduct>, 
                    requestor: &UniquePtr<RustRequestor>, request_id: &CxxString, timeout_milliseconds: i32, domain: &CxxString) -> GReturnCode;

        #[rust_name = "request_sync"]
        fn rustRequestSync(gn: &UniquePtr<GNode>, service_id: &CxxString, request: &UniquePtr<GDataProduct>, timeout_milliseconds: i32, domain: &CxxString) -> SharedPtr<GDataProduct>;

        #[rust_name = "register_service"]
        fn rustRegisterService(gn: &UniquePtr<GNode>, service_id: &CxxString, transport_type: GTransportType, server: &UniquePtr<RustServiceProvider>) -> GReturnCode;
        
        #[rust_name = "unregister_service"]
        fn rustUnregisterService(gn: &UniquePtr<GNode>, service_id: &CxxString) -> GReturnCode;
        
        #[rust_name = "register_relay"]
        fn rustRegisterRelay(gn: &UniquePtr<GNode>, data_product_id: &CxxString, subscriber: &UniquePtr<RustSubscriber>,
                             local_only: bool, transport_type: GTransportType) -> GReturnCode;

        #[rust_name = "register_relay_cache"]
        fn rustRegisterRelayCache(gn: &UniquePtr<GNode>, data_product_id: &CxxString, subscriber: &UniquePtr<RustSubscriber>,
                             local_only: bool, transport_type: GTransportType, cache_last_value: bool) -> GReturnCode;

        #[rust_name = "unregister_relay"]
        fn rustUnregisterRelay(gn: &UniquePtr<GNode>, data_product_id: &CxxString,  subscriber: &UniquePtr<RustSubscriber>) -> GReturnCode;

        #[rust_name = "create_future_response"]
        fn rustCreateFutureResponse(gn: &UniquePtr<GNode>) -> SharedPtr<GFutureResponse>;

        #[rust_name = "send_future_response"]
        fn rustSendFutureResponse(gn: &UniquePtr<GNode>, future_response: &SharedPtr<GFutureResponse>) -> GReturnCode;

        #[rust_name = "set_subscription_timeout_monitor"]
        fn rustSetSubscriptionTimeoutMonitor(gn: &UniquePtr<GNode>, data_product_id: &CxxString, monitor: &UniquePtr<RustSubscriptionMonitor>,
            milli_second_timeout: i32, filter: &CxxString, domain: &CxxString) -> GReturnCode;
        
        #[rust_name = "clear_subscription_timeout_monitor"]
        fn rustClearSubscriptionTimeoutMonitor(gn: &UniquePtr<GNode>, data_product_id: &CxxString, monitor: &UniquePtr<RustSubscriptionMonitor>,
            filter: &CxxString, domain: &CxxString) -> GReturnCode;

        // GravityDataProductMethods

        #[rust_name = "gravity_data_product"]
        fn newGravityDataProduct(dataProductId: &CxxString) -> UniquePtr<GDataProduct>;
        
        #[rust_name = "gravity_data_product_default"]
        fn newGravityDataProduct() -> UniquePtr<GDataProduct>;

        #[rust_name = "gravity_data_product_bytes"]
        unsafe fn newGravityDataProduct(arrayPtr: *const c_char, size: i32) -> UniquePtr<GDataProduct>;

        #[rust_name = "set_data_basic"]
        unsafe fn rustSetData(gdp: &UniquePtr<GDataProduct>, data: *const c_char, size: i32);
    
        #[rust_name = "set_data"]
        unsafe fn rustSetDataProto(gdp: &UniquePtr<GDataProduct>, data: *const c_char, size: i32, data_type: &CxxString);
    
        #[rust_name = "get_gravity_timestamp"]
        fn rustGetGravityTimestamp(gdp: &UniquePtr<GDataProduct>) -> u64;

        #[rust_name = "get_receieved_timestamp"]
        fn rustGetReceivedTimestamp(gdp: &UniquePtr<GDataProduct>) -> u64;

        #[rust_name = "get_data_product_ID"]
        fn rustGetDataProductID(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "set_software_version"]
        fn rustSetSoftwareVersion(gdp: &UniquePtr<GDataProduct>, softwareVersion: &CxxString);

        #[rust_name = "get_software_version"]
        fn rustGetSoftwareVersion(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "get_data_size"]
        fn rustGetDataSize(gdp: &UniquePtr<GDataProduct>) -> i32;

        #[rust_name = "get_data"]
        fn rustGetData(gdp: &UniquePtr<GDataProduct>) -> * const c_char;

        #[rust_name = "get_size"]
        fn rustGetSize(gdp: &UniquePtr<GDataProduct>) -> i32;

        #[rust_name = "parse_from_array"]
        unsafe fn rustParseFromArray(gdp: &UniquePtr<GDataProduct>, array_ptr: * const c_char, size: i32);

        #[rust_name = "serialize_to_array"]
        fn rustSerializeToArray(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxVector<u8>>;

        #[rust_name = "gdp_get_component_id"]
        fn rustGDPGetComponentID(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "gdp_get_domain"]
        fn rustGetDomain(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "is_future_response"]
        fn rustIsFutureResponse(gdp: &UniquePtr<GDataProduct>) -> bool;

        #[rust_name = "is_cached_data_product"]
        fn rustIsCachedDataProduct(gdp: &UniquePtr<GDataProduct>) -> bool;

        #[rust_name = "set_is_cached_data_product"]
        fn rustSetIsCachedDataProduct(gdp: &UniquePtr<GDataProduct>, cached: bool);

        #[rust_name = "get_future_socket_url"]
        fn rustGetFutureSocketURL(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "set_timestamp"]
        fn rustSetTimestamp(gdp: &UniquePtr<GDataProduct>, ts: u32);

        #[rust_name = "set_recieved_timestamp"]
        fn rustSetReceivedTimestamp(gdp: &UniquePtr<GDataProduct>, ts: u32);

        // #[rust_name = "set_component_id"]
        // fn rustSetComponentId(gdp: &UniquePtr<GDataProduct>, component_id: &CxxString);

        // #[rust_name = "set_domain"]
        // fn rustSetDomain(gdp: &UniquePtr<GDataProduct>, domain: &CxxString);

        #[rust_name = "is_relayed_data_product"]
        fn rustIsRelayedDataProduct(gdp: &UniquePtr<GDataProduct>) -> bool;

        #[rust_name = "set_is_relayed_data_product"]
        fn rustSetIsRelayedDataProduct(gdp: &UniquePtr<GDataProduct>, relayed: bool);

        #[rust_name = "set_protocol"]
        fn rustSetProtocol(gdp: &UniquePtr<GDataProduct>, protocol: &CxxString);

        #[rust_name = "get_protocol"]
        fn rustGetProtocol(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "set_type_name"]
        fn rustSetTypeName(gdp: &UniquePtr<GDataProduct>, data_type: &CxxString);

        #[rust_name = "get_type_name"]
        fn rustGetTypeName(gdp: &UniquePtr<GDataProduct>) -> UniquePtr<CxxString>;

        #[rust_name = "get_registration_time"]
        fn rustGetRegistrationTime(gdp: &UniquePtr<GDataProduct>) -> u32;

        #[rust_name = "set_registration_time"]
        fn rustSetRegistrationTime(gdp: &UniquePtr<GDataProduct>, ts: u32);
        
        #[rust_name = "copy_gdp"]
        fn copyGravityDataProduct(gdp: &GDataProduct) -> UniquePtr<GDataProduct>;

        #[rust_name = "copy_gdp_shared"]
        fn copyGravityDataProductShared(gdp: &GDataProduct) -> SharedPtr<GDataProduct>;
    
        #[rust_name ="free_data"]
        unsafe fn rustFree(data: * const c_char);


        // Future response
        #[rust_name = "new_future_response"]
        unsafe fn rustNewFutureResponse(array_ptr: * const c_char, size: i32) -> SharedPtr<GFutureResponse>;
    
        #[rust_name = "set_response"]
        fn rustSetResponse(fr: &UniquePtr<GFutureResponse>, respons: &UniquePtr<GDataProduct>);
    
    
    }


    unsafe extern "C++" {
        //spdlog
        include!("gravity/lib/RustSpdLog.h");
        
        fn spdlog_critical(message: &CxxString);
        fn spdlog_error(message: &CxxString);
        fn spdlog_warn(message: &CxxString);
        fn spdlog_info(message: &CxxString);
        fn spdlog_debug(message: &CxxString);
        fn spdlog_trace(message: &CxxString);

    }

    // extern "Rust" {
    //     #[cxx_name = "RustDataProduct"]
    //     type GravityDataProduct;
    // }
} // mod ffi


