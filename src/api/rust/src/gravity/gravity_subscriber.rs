use crate::gravity::gravity_data_product::GravityDataProduct;


pub trait GravitySubscriber {
    fn subscription_filled(&self, data_products: &Vec<GravityDataProduct>);
}
