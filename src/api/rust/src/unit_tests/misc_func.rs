use core::time;

use crate::{gravity_data_product::GravityDataProduct, gravity_heartbeat_listener::GravityHeartbeatListener, gravity_node::{GravityNode, GravityTransportType}, gravity_subscriber::GravitySubscriber, spdlog::SpdLog};



fn misc_component_1 () -> GravityNode {
    let gn = GravityNode::new();

    gn.init("MiscGravityComponentID1");

    let interval = 500000;

    gn.start_heartbeat(interval);

    gn.register_data_product("IPCDataProduct", GravityTransportType::TCP);

    let mut count = 5;
    while count > 0 {
        count -= 1;


        let ipc_data_product = GravityDataProduct::from_id("IPCDataProduct");

        let data = "hey!";

        ipc_data_product.set_data_basic(data.as_bytes());

        gn.publish(&ipc_data_product);


    }
    gn
}


struct MiscHBListener {}

impl GravityHeartbeatListener for MiscHBListener {
    fn missed_heartbeat(&mut self, component_id: &str, microsecond_to_last_heartbeat: i64, 
        interval_in_microseconds: &mut i64) {
        // SpdLog::warn(format!("Missed Heartbeat. Last one {}  microseconds agpo", microsecond_to_last_heartbeat));
    }

    fn received_heartbeat(&mut self, component_id: &str, interval_in_microseconds: &mut i64) {
        // SpdLog::warn("Received heartbeat");
    }
}

struct MiscGravitySubscriber {}

impl GravitySubscriber for MiscGravitySubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        for data_product in data_products {
            let data = data_product.get_data();
            let message = String::from_utf8(data).unwrap();
            assert_eq!(String::from("hey!"), message);
            // SpdLog::warn(format!("Got message {}", message));
        }
    }
}

fn misc_component_2 () -> GravityNode {
    let mut gn = GravityNode::new();
    gn.init("MiscGravityComponentID2");

    let interval = 500000;

    let hb1 = MiscHBListener {};

    gn.register_heartbeat_listener("MiscGravityComponentID1", interval, &hb1);


    let ipc_subscriber = MiscGravitySubscriber {};

    gn.subscribe("IPCDataProduct", &ipc_subscriber);

    gn
}

#[test]
fn misc_func () {

    // test if the listener drops first (good now)
    let listener = misc_component_2();
    // misc_component_2();

    let beater = misc_component_1();
    // std::mem::drop(beater);

    std::thread::sleep(time::Duration::from_secs(3));
    // std::mem::drop(beater);
    // std::mem::drop(listener);
    std::thread::sleep(time::Duration::from_secs(3));

}

#[test] 
fn listener_drops_second () {
    // test if the listener drops second (good)
    let listener = misc_component_2();

    let beater = misc_component_1();

    std::thread::sleep(time::Duration::from_secs(3));

    listener.stop_heartbeat(); //does nothing, but need an action to  keep it around

}