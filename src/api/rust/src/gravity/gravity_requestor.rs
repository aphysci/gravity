
use crate::gravity::gravity_data_product::GravityDataProduct;


pub trait GravityRequestor {
    fn request_filled(&self, service_id: String, request_id: String, response: &GravityDataProduct);
}