use core::{ffi::c_void, time};
use std::ffi::c_char;
use protobuf::Message;

pub use ffi::*;
use cxx::{let_cxx_string, CxxString, UniquePtr};


#[cxx::bridge]
mod ffi {
    #[namespace = "gravity"]
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
    enum GravityTransportType {
        TCP = 0,     ///< Transmission Control Protocol
        INPROC = 1,  ///< In-process (Inter-thread) Communication
        PGM = 2,     ///< Pragmatic General Multicast Protocol
        EPGM = 3,    ///< Encapsulated PGM
        IPC = 4,  //< Inter-Process Communication
    }
    #[namespace = "gravity"]
    unsafe extern "C++" {
        include!("/home/anson/gravity/src/api/rust/lib/GravityNode.h");
        include!("/home/anson/gravity/src/api/rust/lib/GravityDataProduct.h");
        include!("/home/anson/gravity/src/api/rust/lib/SpdLog.h");

        type GravityReturnCode;
        type GravityTransportType;
        type GravityNode; 
        type GravityDataProduct;

        //GravityNode methods
        #[rust_name = "GravityNode"]
        fn newGravityNode() -> UniquePtr<GravityNode>;

        #[rust_name = "init"]
        fn rustInit(gn: &UniquePtr<GravityNode>, componentID: &CxxString) -> GravityReturnCode;
        
        #[rust_name = "getComponentID"]
        fn rustGetComponentID(gn: &UniquePtr<GravityNode>) -> UniquePtr<CxxString>;
        
        #[rust_name = "registerDataProduct"]
        fn rustRegisterDataProduct(gn: &UniquePtr<GravityNode>, dataProductID: &CxxString, 
            transportType: GravityTransportType) -> GravityReturnCode;
        #[rust_name = "publish"]
        fn rustPublish(gn: &UniquePtr<GravityNode>, dataProduct: &UniquePtr<GravityDataProduct>) -> GravityReturnCode;
        
        // GravityDataProductMethods
        #[rust_name = "GravityDataProduct"]
        fn newGravityDataProduct(dataProductId: &CxxString) -> UniquePtr<GravityDataProduct>;
        
        #[rust_name = "setData"]
        unsafe fn rustSetData(gdp: &UniquePtr<GravityDataProduct>, data: *const c_char, size: i32);
    
        #[rust_name = "setDataProto"]
        unsafe fn rustSetDataProto(gdp: &UniquePtr<GravityDataProduct>, data: *const c_char, size: i32);
        
    }

}