
use crate::gravity_data_product::GravityDataProduct;


pub trait GravityRequestor {
    fn request_filled(&self, service_id: &str, request_id: &str, response: &GravityDataProduct);

    fn request_timeout(&self, service_id: &str, request_id: &str);
}