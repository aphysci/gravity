#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_assignments)]
#![allow(dead_code)]
#![allow(unused_imports)]

// use std::os::raw::c_void;
use core::{ffi::c_void, time};
use std::ffi::c_char;


use cxx::{let_cxx_string, CxxString, UniquePtr};

use crate::ffi::{GravityDataProduct, GravityNode, GravityReturnCode, GravityTransportType};
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
    
        // #[rust_name = "setDataProto"]
        // fn rustSetDataProto(gdp: &UniquePtr<GravityDataProduct>, data: &);
        
    }

}




struct DataProduct {
    gdp: UniquePtr<GravityDataProduct>,
}
struct Node {
    gn: UniquePtr<GravityNode>,
}

impl Node {
    pub fn new() -> Node {
        return Node { gn: ffi::GravityNode() }
    }
    pub fn init(&self, componentID: impl AsRef<[u8]>) -> GravityReturnCode {
        let_cxx_string!(cid = componentID);
        ffi::init(&self.gn, &cid)
    }
    pub fn getComponentID(&self) -> String  
    { (*ffi::getComponentID(&self.gn)).to_str().unwrap().to_string()} 

    pub fn registerDataProduct(&self, dataProductID: impl AsRef<[u8]>, transportType: GravityTransportType) -> GravityReturnCode
    {
        let_cxx_string!(dpid = dataProductID);
        ffi::registerDataProduct(&self.gn, &dpid, transportType)
    }
    pub fn publish(&self, dataProduct: DataProduct) -> GravityReturnCode
    {
        ffi::publish(&self.gn, &(dataProduct.gdp))
    }
    
    
}

impl DataProduct {
    pub fn new(dataProductId: impl AsRef<[u8]>) ->DataProduct {
        let_cxx_string!(dpid = dataProductId);
        DataProduct {gdp: ffi::GravityDataProduct(&dpid)}
    }

    pub fn setData(&self, data: &str, size:i32) {
        let d = data as *const _ as *const c_char;
        unsafe { ffi::setData(&self.gdp, d, size); }
    }
}

fn main() {

    let gn = Node::new();
    let mut ret = gn.init("RustNode");
    while ret != GravityReturnCode::SUCCESS {
        // spdlog::critical("Unable to initialize GravityNode. Retrying...");
        std::thread::sleep(std::time::Duration::from_secs(1));
    }

    println!("Gravity returned code SUCCESS. GravityNode successfullly initialized");
    println!("ID = {}", gn.getComponentID());

    let dataProductID = "RustDataProduct";

    ret = gn.registerDataProduct(&dataProductID, GravityTransportType::TCP);
    if ret != GravityReturnCode::SUCCESS {
        // spdlog::critical("Unable to register data product");
        std::process::exit(1)
    }

    let mut quit = false;
    let mut count = 1;
    while !quit
    {   
        std::thread::sleep(time::Duration::from_secs(1));

        let gdp = DataProduct::new(&dataProductID);

        let mut data = "HelloRustWorld #".to_owned();
        data.push_str(&count.to_string());

        //TODO, but that should be all
        gdp.setData(&data, data.len() as i32);

        ret = gn.publish(gdp);
        if ret != GravityReturnCode::SUCCESS {
            // spdlog::error("Could not publish datat product");
            std::process::exit(1)
        }

        if count == 20 { quit = true;}
        count += 1;
    }
}
