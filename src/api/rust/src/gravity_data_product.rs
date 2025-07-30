#![allow(dead_code)]
use std::ffi::c_char;

use cxx::{let_cxx_string, UniquePtr};
use crate::ffi::{self, *};

/// Generic Data Product for the Gravity Infrastructure.
pub struct GravityDataProduct {
    pub(crate) gdp: UniquePtr<GDataProduct>,
}


impl GravityDataProduct {
    /// Constructs a GravityDataProduct using the default constructor
    /// As of right now, you cannot later set a data product ID for a GravityDataProduct
    /// so use of this function is not recommended
    pub fn new() -> GravityDataProduct {
        GravityDataProduct { gdp: ffi::gravity_data_product_default() }
    }

    /// Constructs a GravityDataProduct with a string as a parameter
    /// Recommended constructor.
    pub fn with_id(data_product_id: &str) -> GravityDataProduct {
        let_cxx_string!(dpid = data_product_id);
        GravityDataProduct {gdp: ffi::gravity_data_product(&dpid)}
    }

    /// Constructor that deserializes this GravityDataProduct from an array of bytes
    pub fn with_array(array: &[u8]) -> GravityDataProduct {
        let size = array.len() as i32;
        let array_ptr = array as *const _ as * const c_char;
        GravityDataProduct {gdp: unsafe {ffi::gravity_data_product_bytes(array_ptr, size)}}
    }


    pub(crate) fn from_gdp(gdp: UniquePtr<GDataProduct>) -> GravityDataProduct {
        GravityDataProduct {gdp}
    }
    /// Returns the data product ID of this data product.
    pub fn data_product_id(&self) -> String {
        ffi::get_data_product_ID(&self.gdp).to_str().unwrap().to_string()
    }
    /// Sets the software version of this data product.
    pub fn set_software_version(&self, software_version: &str)
    {
        let_cxx_string!(sv = software_version);
        ffi::set_software_version(&self.gdp, &sv);
    }
    /// Returns the software version of this data product.
    pub fn software_version(&self) -> String {
        ffi::get_software_version(&self.gdp).to_str().unwrap().to_string()
    }
    /// Returns the received timestamp of the data product.
    pub fn receieved_timestamp(&self) -> u64 {
        ffi::get_receieved_timestamp(&self.gdp)
    }
    /// Returns the gravity timestamp of the data product.
    pub fn gravity_timestamp(&self) -> u64{
        ffi::get_gravity_timestamp(&self.gdp)
    }
    /// Sets the application-specific data for this data product from an array of bytes.
    pub fn set_data_basic(&self, data: &[u8]) {
        let size = data.len() as i32;
        let d = data as *const _ as *const c_char;
        unsafe { ffi::set_data_basic(&self.gdp, d, size); }
    }
    /// Sets the applicatio-specific data for this data product from a protocol buffer.
    pub fn set_data<A: protobuf::Message>(&self, data: &A) {
        let v = data.write_to_bytes().unwrap();
        let bytes = v.as_ptr() as *const c_char;
        let type_name = <A as protobuf::Message>::NAME;
        let_cxx_string!(data_type = type_name);
        unsafe {ffi::set_data(&self.gdp, bytes, data.compute_size() as i32, &data_type);}
    }
    /// Returns the data in this data product as a vector of bytes.
    pub fn data(&self) -> Vec<u8> {
        let data_str = ffi::get_data(&self.gdp);
        let mut pointer = data_str as * const c_char as * const u8;
        let temp = pointer as * const i8;
        let size = ffi::get_data_size(&self.gdp);

        
        let mut bytes_buf = Vec::new();
        for _ in 0..size {
            unsafe {
                bytes_buf.push(*pointer);
                pointer = pointer.offset(1);
            }
            
        }
        unsafe { ffi::free_data(temp); }
        
        bytes_buf
        
    }
    /// Returns the size of the data contained within this data product.
    pub fn data_size(&self) -> i32 {
        ffi::get_data_size(&self.gdp)
    }
    /// Populate a protobuf object with the data conatined in this data product.
    pub fn populate_message(&self, data: &mut impl protobuf::Message) -> bool{
        let temp = self.data();
        let bytes = temp.as_slice();
        // let smth = GravityDataProductPB::parse_from_bytes(bytes).unwrap();
        data.merge_from_bytes(bytes).unwrap();
        true
    }
    /// Returns the size for this message.
    pub fn size(&self) -> i32 {
        ffi::get_size(&self.gdp)
    }
    /// Deserialize this GravityDataProduct from an array of bytes.
    pub fn parse_from_array(&self, array: &[u8]) {
        let len = array.len();
        let p = array.as_ptr() as * const c_char;
        unsafe { ffi::parse_from_array(&self.gdp, p, len as i32);}
    }
    /// Serialize this GravityDatProduct
    pub fn serialize_to_array(&self) -> Vec<u8> {
        let temp = ffi::serialize_to_array(&self.gdp);
        let mut ret = Vec::new();
        for item in temp.iter() {
            ret.push(*item);
        }
        ret
         // think about changing array -> string
    }   
    /// Returns the component ID of the GravityNode that produced this data product
    pub fn component_id(&self) -> String {
        let temp = ffi::gdp_get_component_id(&self.gdp);
        temp.to_str().unwrap().to_string()
    }
    /// Returns the domain of the GravityNode that produced this data product
    pub fn domain(&self) -> String {
        let temp = ffi::gdp_get_domain(&self.gdp);
        temp.to_str().unwrap().to_string()
    }
    /// Returns the flag indicating if this is a "future" response.
    pub fn is_future_response(&self) -> bool {
        ffi::is_future_response(&self.gdp)
    }
    /// Returns the flag indicating if this was a cached data product.
    pub fn is_cached_data_product(&self) -> bool {
        ffi::is_cached_data_product(&self.gdp)
    }
    /// Sets the flag indicating this data product is cached
    pub fn set_is_cached_data_product(&self, cached: bool) {
        ffi::set_is_cached_data_product(&self.gdp, cached);
    } 
    /// Returns the url for the REP socket of a future response
    pub fn future_socket_url(&self) -> String {
        let temp = ffi::get_future_socket_url(&self.gdp);
        temp.to_str().unwrap().to_string()
    }
    /// Sets the timestamp on this GravityDataProduct (typically set by infrastructure at publish)
    pub fn set_timestamp(&self, ts: u32) {
        ffi::set_timestamp(&self.gdp, ts);
    }
    /// Sets the received timestamp on this GravityDataProduct (typically set by infrastructure at publish)
    pub fn set_recieved_timestamp(&self, ts: u32) {
        ffi::set_recieved_timestamp(&self.gdp, ts);
    }

    /// Sets the component id of this GravityDataProduct (typically set by infrastructure at publish)
    pub fn set_component_id(&self, component_id: &str) {
        let_cxx_string!(cid = component_id);
        ffi::set_component_id(&self.gdp, &cid);
    }

    /// Sets the domain on this GravityDataProduct (typically set by infrastructure at publish)
    pub fn set_domain(&self, domain: &str) {
        let_cxx_string!(d = domain);
        ffi::set_domain(&self.gdp, &d);
    }
    /// Returns the flag indicating whether this message has been relayed or has come from the original source
    pub fn is_relayed_data_product(&self) -> bool {
        ffi::is_relayed_data_product(&self.gdp)
    }
    // Sets the flag indicating whether this messsage has been relayed or has come from the original source.
    pub fn set_is_relayed_data_product(&self, relayed: bool) {
        ffi::set_is_relayed_data_product(&self.gdp, relayed);
    }
    /// Sets the data protocol in this data product (for instance, protobuf2).
    pub fn set_protocol(&self, protocol: &str) {
        let_cxx_string!(p = protocol);
        ffi::set_protocol(&self.gdp, &p);
    }
    /// Returns the data protocol in this data product (for instance, protobuf2).
    pub fn protocol(&self) -> String {
        let temp = ffi::get_protocol(&self.gdp);
        temp.to_str().unwrap().to_string()
    }
    /// Sets the type of data in this data product (for instance, the full protobuf type name)
    pub fn set_type_name(&self, data_type: &str) {
        let_cxx_string!(dt = data_type);
        ffi::set_type_name(&self.gdp, &dt);
    }
    /// Returns the type of data in this data product (for instance, the full protobuf type name).
    pub fn type_name(&self) -> String {
        let temp = ffi::get_type_name(&self.gdp);
        temp.to_str().unwrap().to_string()
    }
    /// Returns the time associated with the registration of this data product.
    pub fn registration_time(&self) -> u32 {
        ffi::get_registration_time(&self.gdp)
    }
    /// Sets the registration time on this GravityDataProduct (typically set by infrastructure when created).
    pub fn set_registration_time(&self, ts: u32) {
        ffi::set_registration_time(&self.gdp, ts);
    }
}