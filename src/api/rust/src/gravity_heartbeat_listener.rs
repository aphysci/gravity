/// Interface specification for a trait object that will respond to connection outages of gravity products.
pub trait GravityHeartbeatListener {

    /// Called when another component's heartbeat is off by a certain amount.
    fn missed_heartbeat(&mut self, component_id: &str, microsecond_to_last_heartbeat: i64, 
        interval_in_microseconds: &mut i64);

    /// Called when another component's heartbeat is received.
    fn received_heartbeat(&mut self, component_id: &str, interval_in_microseconds: &mut i64);
} 