use std::ffi::c_char;
use crate::ffi;

use cxx::{let_cxx_string, UniquePtr};

use crate::ffi::*;



pub struct DataProduct {
    gdp: UniquePtr<GravityDataProduct>,
}

pub struct Node {
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
    pub fn setDataProto(&self, data: impl protobuf::Message) {
        let v = data.write_to_bytes().unwrap();
        let bytes = v.as_ptr() as *const c_char;

        unsafe {ffi::setDataProto(&self.gdp, bytes, data.compute_size() as i32);}
    }
}
