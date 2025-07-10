use crate::gravity_data_product::GravityDataProduct;

// #[derive(Copy)]
pub trait GravitySubscriber {
    fn subscription_filled(&self, data_products: &Vec<GravityDataProduct>);
}
