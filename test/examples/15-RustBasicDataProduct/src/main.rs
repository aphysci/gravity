include!(concat!(env!("OUT_DIR"), "/protobuf/mod.rs"));

use core::time;

use gravity::{GravityReturnCode, GravityDataProduct, GravityNode, GravitySubscriber, GravityTransportType, SpdLog};
use BasicCounterDataProduct::*;


struct SimpleGravityCounterSubscriber {}

impl GravitySubscriber for SimpleGravityCounterSubscriber {
    fn subscription_filled(&mut self, data_products: Vec<GravityDataProduct>) {
        
        for data_product in data_products.iter() {
            let mut counter_data_pb = BasicCounterDataProductPB::new();
            data_product.populate_message(&mut counter_data_pb);

            SpdLog::warn(format!("Current Count: {}", counter_data_pb.count()));

        }
    }
}
fn main() {
    // SpdLog::warn("Starting...");
    let mut gn = GravityNode::new();
    while gn.init("BasicCounterSubscriberID") != GravityReturnCode::SUCCESS {
	    SpdLog::warn("retrying init");
    }
    assert_eq!(1234, gn.get_int_param("test_number", 0));

    gn.register_data_product("BasicCounterDataProduct", GravityTransportType::TCP);
    let counter_subscriber = gn.tokenize_subscriber(SimpleGravityCounterSubscriber {});

    gn.subscribe("BasicCounterDataProduct", &counter_subscriber);

    
    //setup the publisher
    let mut count = 1;
    while count <= 20 {
        let mut counter_pb = BasicCounterDataProductPB::new();
        counter_pb.set_count(count);

        let mut gdp = GravityDataProduct::with_id("BasicCounterDataProduct");
        gdp.set_data(&counter_pb);
        gn.publish(&gdp);

        count += 1;

        std::thread::sleep(time::Duration::from_millis(1000));
    }
    
}
