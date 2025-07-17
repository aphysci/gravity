
use crate::gravity_data_product::GravityDataProduct;


pub trait GravityRequestor {
    fn request_filled(&mut self, service_id: &str, request_id: &str, response: &GravityDataProduct);

    fn request_timeout(&mut self, service_id: &str, request_id: &str);
}