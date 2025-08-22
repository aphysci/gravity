use crate::gravity_data_product::GravityDataProduct;

/// Interface specification for a trait object that will function as the "server" side of a request-response interaction.
pub trait GravityServiceProvider {
    /// Called when a request is made through the Gravity infrastructure
    fn request(&mut self, service_id: &str, data_product: &GravityDataProduct) -> GravityDataProduct;
}

pub struct ServiceWrap {
    pub(crate) service: Box<dyn GravityServiceProvider>,
}