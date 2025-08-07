/// Interface specification for a trait object that will respons to subscription timeouts.
pub trait GravitySubscriptionMonitor {
    /// Called on implementing object when a subscription is not received within the registered time constraints.
    fn subscription_timeout(&mut self, data_product_id: &str, milli_seconds_since_last: i32,
         filter: &str, domain: &str);

}

pub(crate) struct MonitorWrap {
    
}