mod protos;
mod gravity;

use std::time;
use spdlog::prelude::*;
use crate::gravity::gravity::*;
use crate::protos::DataPB::MultPB;

struct MySubscriber {}

impl GravitySubscriber for MySubscriber {
    fn subscription_filled(&self, data_products: &Vec<GravityDataProduct>) {
        for i in data_products.iter() {
           let mut pb = MultPB::new();
           i.populate_message(&mut pb);
           warn!("op1, op2: {}, {}", pb.multiplicand_a.unwrap(), pb.multiplicand_b.unwrap());
        }
    }
}
fn main() {
    let mut gnn = GravityNode::new();
    gnn.init("SubNode");
    let data_product_id = "RustDataProduct";

    let subscriber = MySubscriber {};

    gnn.subscribe(data_product_id, &subscriber);

    
    GravityLogger::info("Beginning rust version of gravity");

    let gn = GravityNode::new();
    let mut ret = gn.init("RustNode");
    if ret != GravityReturnCode::SUCCESS {
        GravityLogger::critical(format!("Unable to initialize GravityNode (return code {:?})", ret));
        std::process::exit(1);
    }
    // info!("Gravity returned code SUCCESS. Init successful");


    
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


        let mut data = MultPB::new();
        data.set_multiplicand_a(count);
        data.set_multiplicand_b(count + 1048576);

    //     //TODO, but that should be all
        gdp.set_data(&data);
        
        ret = gn.publish(gdp);
        if ret != GravityReturnCode::SUCCESS {
            GravityLogger::error(format!("Could not publish data product (return code {:?})", ret));
            std::process::exit(1)
        }
    

        if count == 15 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_secs(1));
    }
    // gn.wait_for_exit();  // Dont want this here because i will kill process anyway
   
}
