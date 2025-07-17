use crate::gravity_data_product::GravityDataProduct;

// #[derive(Copy)]
pub trait GravitySubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>);
}
