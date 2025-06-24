use core::{ffi::c_void, time};
use std::{ffi::c_char, ops::Sub};
use protobuf::Message;

pub use ffi::*;
use cxx::{let_cxx_string, CxxString, CxxVector, UniquePtr};
use autocxx::{prelude::*, subclass::subclass};

use crate::gravity::GraDataProduct;


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

        type GravityDataProduct;
 
        //GravityNode methods
        #[rust_name = "GravityNode"]
        fn newGravityNode() -> UniquePtr<GNode>;

        #[rust_name = "GravityNodeID"]
        fn newGravityNode(componentID: &CxxString) -> UniquePtr<GNode>;

        #[rust_name = "init"]
        fn rustInit(gn: &UniquePtr<GNode>, componentID: &CxxString) -> GravityReturnCode;
        
        #[rust_name = "init_default"]
        fn rustInit(gn: &UniquePtr<GNode>) -> GravityReturnCode;

        #[rust_name = "wait_for_exit"]
        fn rustWaitForExit(gn: &UniquePtr<GNode>);

        #[rust_name = "getComponentID"]
        fn rustGetComponentID(gn: &UniquePtr<GNode>) -> UniquePtr<CxxString>;
        
        #[rust_name = "registerDataProduct"]
        fn rustRegisterDataProduct(gn: &UniquePtr<GNode>, dataProductID: &CxxString, 
            transportType: GravityTransportType) -> GravityReturnCode;
        #[rust_name = "publish"]
        fn rustPublish(gn: &UniquePtr<GNode>, dataProduct: &UniquePtr<GravityDataProduct>) -> GravityReturnCode;
        
        #[rust_name = "subsribers_exist"]
        fn rustSubscribersExist(gn: &UniquePtr<GNode>, dataProductID: &CxxString, has_subscribers: &mut bool) -> GravityReturnCode;
        // #[rust_name = "subscribe"]
        // fn rustSubscribe(gn: &UniquePtr<GNode>, dataProductID: &CxxString, subscriber: &RustSubscriber) -> GravityReturnCode;

        // GravityDataProductMethods
        #[rust_name = "GravityDataProduct"]
        fn newGravityDataProduct(dataProductId: &CxxString) -> UniquePtr<GravityDataProduct>;
        
        #[rust_name = "setDataBasic"]
        unsafe fn rustSetData(gdp: &UniquePtr<GravityDataProduct>, data: *const c_char, size: i32);
    
        #[rust_name = "setData"]
        unsafe fn rustSetDataProto(gdp: &UniquePtr<GravityDataProduct>, data: *const c_char, size: i32);
    
            
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

