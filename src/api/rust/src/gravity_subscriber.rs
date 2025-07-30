use crate::gravity_data_product::GravityDataProduct;

/// Interface specification for an object that will respond to subscriptions.
pub trait GravitySubscriber {
    /// Called on implementing trait object when a registered subscription is filled
    /// with 1 or more GravityDataProducts.
    fn subscription_filled(&mut self, data_products: Vec<GravityDataProduct>);
}
