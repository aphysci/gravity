#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_assignments)]
#![allow(dead_code)]
#![allow(unused_imports)]

mod gravity;
mod ffi1;
mod protos;
use std::fmt::format;
use std::time;
use cxx::CxxVector;
use spdlog::prelude::*;

use crate::gravity::*;
use crate::protos::DataPB::MultPB;

struct MySubscriber {}

impl GravitySubscriber for MySubscriber {
    fn subscriptionFilled(&self, dataProducts: &Vec<GravityDataProduct>) {
        for i in dataProducts.iter() {
           let mut pb = MultPB::new();
           i.populate_message(&mut pb);
           warn!("op1, op2: {}, {}", pb.multiplicand_a.unwrap(), pb.multiplicand_b.unwrap());
        }
    }
}
fn main() {
    
    gravity_logger::info("Beginning rust version of gravity");

    let mut gn = GravityNode::new();
    let mut ret = gn.init("RustNode");
    if ret != GravityReturnCode::SUCCESS {
        gravity_logger::critical(format!("Unable to initialize GravityNode (return code {:?})", ret));
        std::process::exit(1);
    }
    // info!("Gravity returned code SUCCESS. Init successful");


    let dataProductID = "RustDataProduct";

    ret = gn.register_data_product(&dataProductID, GravityTransportType::TCP);
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to register data product (return code {:?})", ret);
        std::process::exit(1)
    }

    let subscriber = MySubscriber {};

    ret = gn.subscribe(dataProductID, &subscriber);

    std::thread::sleep(time::Duration::from_secs(1));

    let mut quit = false;
    let mut count = 1;
    while !quit
    {   
        

        let gdp = GravityDataProduct::from_id(&dataProductID);


        let mut data = MultPB::new();
        data.set_multiplicand_a(count);
        data.set_multiplicand_b(count + 1);

    //     //TODO, but that should be all
        gdp.set_data(&data);
        
        ret = gn.publish(gdp);
        if ret != GravityReturnCode::SUCCESS {
            gravity_logger::error(format!("Could not publish data product (return code {:?})", ret));
            std::process::exit(1)
        }
    

        if count == 15 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_secs(1));
    }
    // gn.wait_for_exit();  // Dont want this here because i will kill process anyway
   
}
