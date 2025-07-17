
pub trait GravityHeartbeatListener {
    fn missed_heartbeat(&mut self, component_id: &str, microsecond_to_last_heartbeat: i64, 
        interval_in_microseconds: &mut i64);

    fn received_heartbeat(&mut self, component_id: &str, interval_in_microseconds: &mut i64);
} 