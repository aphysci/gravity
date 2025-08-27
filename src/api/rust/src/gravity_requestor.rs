
use crate::gravity_data_product::GravityDataProduct;

/// Interface specification for a trait object that will function as the "client" side of a request-response interaction.
pub trait GravityRequestor {
    /// Called when a response to a request is received through the Gravity infrastructure.
    fn request_filled(&mut self, service_id: &str, request_id: &str, response: &GravityDataProduct);

    /// Called when the response to a request has timed out.
    fn request_timeout(&mut self, service_id: &str, request_id: &str);
}


pub(crate) struct RequestorWrap {
    pub(crate) requestor: Box<dyn GravityRequestor>,
}