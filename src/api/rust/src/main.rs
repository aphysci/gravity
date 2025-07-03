mod protos;
mod gravity;

use std::fmt::format;
use std::time;
use spdlog::prelude::*;
use crate::gravity::gravity::GravityLogger;
use crate::gravity::gravity_data_product::GravityDataProduct;
use crate::gravity::gravity_node::*;
use crate::gravity::gravity_requestor::GravityRequestor;
use crate::gravity::gravity_service_provider::GravityServiceProvider;
use crate::gravity::gravity_subscriber::GravitySubscriber;
use crate::protos::DataPB::{MultPB, ResultPB};

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

struct MyProvider {}

impl GravityServiceProvider for MyProvider {
    fn request(&self, service_id: String, data_product: &GravityDataProduct) -> GravityDataProduct {
        if data_product.get_data_product_id() != String::from("Multiplication") {
            GravityLogger::error("Request is not for multiplication");
        }
        let mut mult_ops = MultPB::new();
        data_product.populate_message(&mut mult_ops);

        GravityLogger::warn(format!("Request recieved {} x {}", mult_ops.multiplicand_a.unwrap(), mult_ops.multiplicand_b.unwrap()));
    
        let result = mult_ops.multiplicand_a.unwrap() * mult_ops.multiplicand_b.unwrap();

        let mut result_pb = ResultPB::new();
        result_pb.set_result(result);

        let ret = GravityDataProduct::from_id("MultiplicationResult");
        ret.set_data(&result_pb);
        ret
    }
}
struct MyRequestor {}

impl GravityRequestor for MyRequestor {
    fn request_filled(&self, service_id: String, request_id: String, response: &GravityDataProduct) {
        let mut result_pb = ResultPB::new();
        response.populate_message(&mut result_pb);

        GravityLogger::warn(format!("Async response recieved: {} = {}", request_id, result_pb.result.unwrap()));
   
    }
}

fn main() {

    let mut gn = GravityNode::new();

    gn.init("MultiplicationComponent");

    let msp = MyProvider {};
    gn.register_service("Multiplication",
     GravityTransportType::TCP, &msp);

     let mut gn2 = GravityNode::new();

     let mut ret = gn2.init("MultiplicationRequestor");

     while ret != GravityReturnCode::SUCCESS {
        GravityLogger::warn("Unable to init component, retrying...");
        ret = gn2.init("MultiplicationRequestor");
     }

    let requestor = MyRequestor {};

    let mult_request = GravityDataProduct::from_id("Multiplication");
    let mut operands = MultPB::new();

    operands.set_multiplicand_a(8);
    operands.set_multiplicand_b(2);
    mult_request.set_data(&operands);
    

    ret = gn2.request_async("Multiplication", &mult_request, &requestor, Some("8 x 2"), None, Some(""));
    while ret != GravityReturnCode::SUCCESS {
        spdlog::warn!("request to Multiplication service failed, retrying...");
        std::thread::sleep(time::Duration::from_secs(1));

        ret = gn2.request_async("Multiplication", &mult_request, &requestor, Some("8 x 2"), None, Some(""));
    }

    // let mut gnn = GravityNode::new();
    // gnn.init("SubNode");
    // let data_product_id = "RustDataProduct";

    // let subscriber = MySubscriber {};

    // gnn.subscribe(data_product_id, &subscriber);

    
    // GravityLogger::info("Beginning rust version of gravity");

    // let gn = GravityNode::new();
    // let mut ret = gn.init("RustNode");
    // if ret != GravityReturnCode::SUCCESS {
    //     GravityLogger::critical(format!("Unable to initialize GravityNode (return code {:?})", ret));
    //     std::process::exit(1);
    // }
    // // info!("Gravity returned code SUCCESS. Init successful");


    
    // ret = gn.register_data_product(&data_product_id, GravityTransportType::TCP);
    // if ret != GravityReturnCode::SUCCESS {
    //     critical!("Unable to register data product (return code {:?})", ret);
    //     std::process::exit(1)
    // }

    

    // std::thread::sleep(time::Duration::from_secs(1));

    // let mut quit = false;
    // let mut count = 1;
    // while !quit
    // {   
    //     let gdp = GravityDataProduct::from_id(&data_product_id);

    //     let mut data = MultPB::new();
    //     data.set_multiplicand_a(count);
    //     data.set_multiplicand_b(count + 1048576);

    // //     //TODO, but that should be all
    //     gdp.set_data(&data);
        
    //     ret = gn.publish(gdp);
    //     if ret != GravityReturnCode::SUCCESS {
    //         GravityLogger::error(format!("Could not publish data product (return code {:?})", ret));
    //         std::process::exit(1)
    //     }
    //     if count == 9 { gnn.unsubscribe(data_product_id, &subscriber); }

    //     if count == 15 { quit = true;}
    //     count += 1;

    //     std::thread::sleep(time::Duration::from_secs(1));
    // }
    gn.wait_for_exit();  // Dont want this here because i will kill process anyway

   
}
