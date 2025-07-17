use crate::gravity_data_product::GravityDataProduct;

pub trait GravityServiceProvider {
    fn request(&mut self, service_id: &str, data_product: &GravityDataProduct) -> GravityDataProduct;
}