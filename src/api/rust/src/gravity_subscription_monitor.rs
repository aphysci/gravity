
pub trait GravitySubscriptionMonitor {
    fn subscription_timeout(&mut self, data_product_id: &str, milli_seconds_since_last: i32,
         filter: &str, domain: &str);

}

