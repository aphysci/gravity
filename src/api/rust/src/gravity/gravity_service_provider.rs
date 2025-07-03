use crate::gravity::gravity_data_product::GravityDataProduct;

pub trait GravityServiceProvider {
    fn request(&self, service_id: String, data_product: &GravityDataProduct) -> GravityDataProduct;
}