
pub trait GravityHeartbeatListener {
    fn missed_heartbeat(&self, component_id: &str, microsecond_to_last_heartbeat: i64, 
        interval_in_microseconds: &mut i64);

    fn recieved_heartbeat(&self, component_id: &str, interval_in_microseconds: &mut i64);
} 