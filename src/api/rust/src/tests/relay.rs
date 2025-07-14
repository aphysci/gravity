use std::{process::Command, time};

use spdlog::{critical, error, warn};

use crate::{gravity_data_product::GravityDataProduct, gravity_node::{GravityNode, GravityReturnCode, GravityTransportType}, gravity_subscriber::GravitySubscriber, protos::BasicCounterDataProduct::BasicCounterDataProductPB};

struct SimpleCounterSubscriber {}

impl GravitySubscriber for SimpleCounterSubscriber {
    fn subscription_filled(&self, data_products: &Vec<crate::gravity_data_product::GravityDataProduct>) {
        for data_product in data_products {
            let mut counter_data_pb = BasicCounterDataProductPB::new();
            data_product.populate_message(&mut counter_data_pb);

            assert!(data_product.is_relayed_data_product(), "Not relayed :/ Did you forget to start the Relay process???");
            // warn!("Current Count: {}. Message was relayed: {}", counter_data_pb.count(),
            //     if data_product.is_relayed_data_product() { "true" } else {"false"})
        }
    }
}
fn publish() {
    let data_product_id = "BasicCounterDataProduct";
    let gn = GravityNode::new();
    let mut ret = gn.init("RustExample");
   
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to initialize GravityNode (return code {:?})", ret);
        std::process::exit(1);
    }
  
    ret = gn.register_data_product(&data_product_id, GravityTransportType::TCP);
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to register data product (return code {:?})", ret);
        std::process::exit(1)
    }

    

    std::thread::sleep(time::Duration::from_secs(1));

    let mut quit = false;
    let mut count = 1;
    while !quit
    {   
        let gdp = GravityDataProduct::from_id(&data_product_id);

        let mut data = BasicCounterDataProductPB::new();
        data.set_count(count);

    //     //TODO, but that should be all
        gdp.set_data(&data);
        
        
        ret = gn.publish(&gdp);
        if ret != GravityReturnCode::SUCCESS {
            error!("Could not publish data product (return code {:?})", ret);
            std::process::exit(1)
        }
        // if count == 9 { gnn.unsubscribe(data_product_id, &sub); 
        //     std::thread::sleep(time::Duration::from_millis(300));}

        if count == 20 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_millis(1000));
    }

}

#[test]
fn simple_relay() {

    // This is a horrible test since you have to manually start the relay
    // 
    let mut gn = GravityNode::new();
    gn.init("Subscriber");

    let counter_subscriber = SimpleCounterSubscriber {};

    gn.subscribe("BasicCounterDataProduct", &counter_subscriber);

    publish();

}