#![allow(dead_code)]
#![allow(unused_variables)]




use std::time;
use crate::spdlog::*;
use crate::gravity_node::*;
use crate::gravity_heartbeat_listener::GravityHeartbeatListener;



struct MyListener {}

impl GravityHeartbeatListener for MyListener {
    fn missed_heartbeat(&mut self, component_id: &str, microsecond_to_last_heartbeat: i64, 
        interval_in_microseconds: &mut i64) {
            // GravityLogger::warn(format!("Missed Heartbeat. Last heartbeat {} microseconds ago", microsecond_to_last_heartbeat));

    }

    fn received_heartbeat(&mut self, component_id: &str, interval_in_microseconds: &mut i64) {
        // GravityLogger::warn("Receieved Heartbeat");
    }


}
#[test]
fn simple_heartbeat_test () {

    let mut gn = GravityNode::new();
    gn.init("HeartbeatExample");

    let interval = gn.get_int_param("HeartbeatInterval", 500000) as i64;
    let listen_for_heartbeat = gn.get_bool_param("HeartbeatListener", true);
    
    gn.start_heartbeat(interval);

    let listener = MyListener {};

    if listen_for_heartbeat {
        gn.register_heartbeat_listener("HeartbeatExample", interval, &listener);
    }

    let gn2 = GravityNode::new();
    gn2.init("HeartbeatExample2");

    let mut quit = false;
    let mut count = 0;
    while !quit {
        std::thread::sleep(time::Duration::from_millis(500));
        count += 1;

        if count == 2 {
            gn.stop_heartbeat();
            // print!("stopping")
        }
        if count == 4 {
            gn.start_heartbeat(interval);
        }
        if count == 6 {
            gn.register_heartbeat_listener("HeartbeatExample2", interval, &listener);
        }
        if count == 8 {
            gn.unregister_heartbeat_listener("HeartbeatExample");
        } 
        if count == 10 {
            gn.unregister_heartbeat_listener("HeartbeatExample2");
        }
        if count == 12 {
            quit = true;
        }
    }

//    gn.wait_for_exit();
}
