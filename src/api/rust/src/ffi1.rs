use core::{ffi::c_void, time};
use std::{ffi::c_char, ops::Sub};
use protobuf::Message;

pub use ffi::*;
use cxx::{let_cxx_string, CxxString, CxxVector, UniquePtr};
use autocxx::{prelude::*, subclass::subclass};

use crate::gravity::GravityDataProduct;


#[cxx::bridge]
mod ffi {

    #[namespace = "gravity"]
    #[repr(i32)]
    #[derive(Debug)]
    enum GravityReturnCode {
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
    enum GravityTransportType {
        TCP = 0,     ///< Transmission Control Protocol
        INPROC = 1,  ///< In-process (Inter-thread) Communication
        PGM = 2,     ///< Pragmatic General Multicast Protocol
        EPGM = 3,    ///< Encapsulated PGM
        IPC = 4,  //< Inter-Process Communication
    }
    #[namespace = "gravity"]
    unsafe extern "C++" {
        include!("/home/anson/gravity/src/api/rust/lib/shims.h");
        // include!("/home/anson/gravity/src/api/rust/lib/RustSubscriber.h");

        type GravityReturnCode;
        type GravityTransportType;
        #[rust_name = "GNode"]
        type GravityNode; 

        #[rust_name = "GDataProduct"]
        type GravityDataProduct;
 
        type RustSubscriber;

        //GravityNode methods
        #[rust_name = "GravityNode"]
        fn newGravityNode() -> UniquePtr<GNode>;

        #[rust_name = "gravity_node_ID"]
        fn newGravityNode(componentID: &CxxString) -> UniquePtr<GNode>;

        #[rust_name = "init"]
        fn rustInit(gn: &UniquePtr<GNode>, componentID: &CxxString) -> GravityReturnCode;
        
        #[rust_name = "init_default"]
        fn rustInit(gn: &UniquePtr<GNode>) -> GravityReturnCode;

        #[rust_name = "wait_for_exit"]
        fn rustWaitForExit(gn: &UniquePtr<GNode>);

        #[rust_name = "publish"]
        fn rustPublish(gn: &UniquePtr<GNode>, dataProduct: &UniquePtr<GDataProduct>) -> GravityReturnCode;

        #[rust_name = "subsribers_exist"]
        fn rustSubscribersExist(gn: &UniquePtr<GNode>, dataProductID: &CxxString, has_subscribers: &mut bool) -> GravityReturnCode;

        #[rust_name = "start_heartbeat"]
        fn rustStartHeartbeat(gn: &UniquePtr<GNode>, interval_in_microseconds: i64) -> GravityReturnCode;

        #[rust_name = "stop_heartbeat"]
        fn rustStopHeartbeat(gn: &UniquePtr<GNode>) -> GravityReturnCode;

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
            transportType: GravityTransportType) -> GravityReturnCode;

        #[rust_name = "register_data_product_cache"]
        fn rustRegisterDataProduct(gn: &UniquePtr<GNode>, dataProductID: &CxxString, 
            transportType: GravityTransportType, cacheLastValue: bool) -> GravityReturnCode;

        #[rust_name = "unregister_data_product"]
        fn rustUnregisterDataProduct(gn: &UniquePtr<GNode>, dataProductID: &CxxString) -> GravityReturnCode;

        #[rust_name = "get_code_string"]
        fn rustGetCodeString(gn: &UniquePtr<GNode>, code: GravityReturnCode) -> UniquePtr<CxxString>;
        
        #[rust_name = "get_IP"]
        fn rustGetIP(gn: &UniquePtr<GNode>) -> UniquePtr<CxxString>;

        #[rust_name = "get_domain"]
        fn rustGetDomain(gn: &UniquePtr<GNode>) -> UniquePtr<CxxString>;

        // #[rust_name = "subscribe"]
        // fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString, subscriber: &RustSubscriber) -> GravityReturnCode;

        // GravityDataProductMethods
        #[rust_name = "GravityDataProduct"]
        fn newGravityDataProduct(dataProductId: &CxxString) -> UniquePtr<GDataProduct>;
        
        #[rust_name = "setDataBasic"]
        unsafe fn rustSetData(gdp: &UniquePtr<GDataProduct>, data: *const c_char, size: i32);
    
        #[rust_name = "setData"]
        unsafe fn rustSetDataProto(gdp: &UniquePtr<GDataProduct>, data: *const c_char, size: i32);
    
        #[rust_name = "subscribe"]
        fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString,  subscriber: &UniquePtr<RustSubscriber>) -> GravityReturnCode;
        
        fn newRustSubscriber(func: fn(&CxxVector<GDataProduct>)) -> UniquePtr<RustSubscriber>;
            
        #[rust_name = "copy_gdp"]
        fn copyGravityDataProduct(gdp: &GDataProduct) -> UniquePtr<GDataProduct>;
    
    }


    unsafe extern "C++" {
        //spdlog
        include!("/home/anson/gravity/src/api/rust/lib/shims.h");
        
        fn spdlog_critical(message: &CxxString);
        fn spdlog_error(message: &CxxString);
        fn spdlog_warn(message: &CxxString);
        fn spdlog_info(message: &CxxString);
        fn spdlog_debug(message: &CxxString);
        fn spdlog_trace(message: &CxxString);
    }

} // mod ffi

