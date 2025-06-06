#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_assignments)]

use cxx::let_cxx_string;

use crate::gravity::GravityReturnCode;
#[cxx::bridge(namespace = "gravity")]
mod gravity {

    
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
    
    unsafe extern "C++" {
        include!("/home/anson/rust-stuff/lib/GravityNode.h");
        type GravityReturnCode;
        type GravityNode; 
        // type GravitySubscriber;
        // type GravityDataProduct;
        #[rust_name = "GravityNode"]
        fn newGravityNode() -> UniquePtr<GravityNode>;
        #[rust_name = "init"]
        fn rustInit(gn: UniquePtr<GravityNode>, componentID: &CxxString, ret: &mut GravityReturnCode) -> UniquePtr<GravityNode>;
        // fn init(componentID: CxxString) -> GravityReturnCode;
        // fn subscribe(dataProductID: CxxString, subscriber: GravitySubscriber) -> GravityReturnCode;
        // fn waitForExit();
        // fn unsubscribe(dataProductID: CxxString, subscriber: GravitySubscriber) -> GravityReturnCode;
        // fn publish(dataProduct: GravityDataProduct) -> GravityReturnCode;
        
    }

}
fn main() {
    
    let_cxx_string!(id = "RustNode");
    let mut ret= gravity::GravityReturnCode::FAILURE;
    let mut gn = gravity::GravityNode();
    gn = gravity::init(gn, &id, &mut ret);
    if ret == GravityReturnCode::SUCCESS {
        println!("SUCCESS");
    } else {
        println!("FAILURE");
    }
}
