#![allow(dead_code)]
#![allow(unused_variables)]


use std::cell::Cell;
use std::sync::WaitTimeoutResult;
use std::time;
use crate::gravity_data_product::GravityDataProduct;
use crate::gravity_node::*;
use crate::gravity_subscriber::GravitySubscriber;
use crate::protos::BasicCounterDataProduct::BasicCounterDataProductPB;
use protobuf::well_known_types::duration;
use spdlog::*;

use crate::protos::DataPB::*;

struct MySubscriber {}

impl GravitySubscriber for MySubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        for i in data_products.iter() {
           let mut pb = MultPB::new();
           i.populate_message(&mut pb);
        //    warn!("got {}, {}", pb.multiplicand_a(), pb.multiplicand_b());
           assert!(pb.multiplicand_b() == (pb.multiplicand_a() + 1024));
        }
    }
}

#[test]
fn basic_subscriber () {

    let mut gnn = GravityNode::new();
    gnn.init("SubNode");
    let data_product_id = "RustDataProduct";

    let subscriber = MySubscriber {};

    gnn.subscribe(data_product_id, &subscriber);

    let gn = GravityNode::new();
    let mut ret = gn.init("RustExample");
    let fs = gn.get_int_param("Fs", 0);
   
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to initialize GravityNode (return code {:?})", ret);
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
        data.set_multiplicand_b(count + 1024);

    //     //TODO, but that should be all
        gdp.set_data(&data);
        
        
        ret = gn.publish(&gdp);
        if ret != GravityReturnCode::SUCCESS {
            error!("Could not publish data product (return code {:?})", ret);
            std::process::exit(1)
        }
        if count == 9 { gnn.unsubscribe(data_product_id, &subscriber); }

        if count == 15 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_millis(100));
    }

    // gn.wait_for_exit();  


   
}


struct CounterSubscriber {
    count_totals: i32,
}

impl GravitySubscriber for CounterSubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        // warn!("Subscriber 1 got message");
        for i in data_products.iter() {
            let mut counter_data_pb = BasicCounterDataProductPB::new();
            i.populate_message(&mut counter_data_pb);
            self.count_totals = self.count_totals + counter_data_pb.count();
            
            // warn!("Subscriber 1: Sum of All Counts Receieved: {}", self.count_totals.get())
            
        }
        
    }
}

struct HelloSubscriber {}


impl GravitySubscriber for HelloSubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        for i in data_products.iter() {
            let message = String::from_utf8(i.get_data()).unwrap();
            // warn!("Subscriber 2: Got message: {}", message);


        }
    }
}

struct SimpleSubscriber {}

impl GravitySubscriber for SimpleSubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        for i in data_products.iter() {
            // warn!("Subscriber 3 got a {} data product", i.get_data_product_id());

            if i.get_data_product_id() == "BasicCounterDataProduct" {
                let mut counter_data_pb = BasicCounterDataProductPB::new();
                i.populate_message(&mut counter_data_pb);
                // warn!("Subscriber3 : Current Count: {}", counter_data_pb.count());

            } else if i.get_data_product_id() == "HelloWorldDataProduct" {
                let message = String::from_utf8(i.get_data()).unwrap();
                // warn!("Subscriber 3: Got message: {}", message);
            }
        }
    }
}
#[test]
fn multiple_subscribers () {

    let mut gn = GravityNode::new();
    gn.init("SimpleGravityComponentSub");

    // subscribe counter
    let counter = CounterSubscriber { count_totals: 0};
    gn.subscribe("BasicCounterDataProduct", &counter);

    // subscribe message
    let hello = HelloSubscriber {};
    gn.subscribe("HelloWorldDataProduct", &hello);

    // subscribe to both
    let simple = SimpleSubscriber {};
    gn.subscribe("BasicCounterDataProduct", &simple);
    gn.subscribe("HelloWorldDataProduct", &simple);

    let gnpub = GravityNode::new();
    gnpub.init("SimpleGravityComponentPub");

    gnpub.register_data_product("BasicCounterDataProduct", GravityTransportType::TCP);
    gnpub.register_data_product("HelloWorldDataProduct", GravityTransportType::TCP);

    let mut quit = false;
    let mut count = 1;

    while !quit 
    {
        let counter_data_product = GravityDataProduct::from_id("BasicCounterDataProduct");
        let mut counter_data_pb = BasicCounterDataProductPB::new();
        counter_data_pb.set_count(count);

        counter_data_product.set_data(&counter_data_pb);

        gnpub.publish(&counter_data_product);

        count += 1;
        if count > 4 { quit = true};


        let hello_data_product = GravityDataProduct::from_id("HelloWorldDataProduct");
        let data = "Hello World";
        hello_data_product.set_data_basic(data.as_bytes());

        gnpub.publish(&hello_data_product);

        // warn!("Published message 1 with count {} and message 2 with data {}", count, data);

        std::thread::sleep(time::Duration::from_secs(1));

    }

    

}

fn external_subscribe(gn: &mut GravityNode, subscriber: MySubscriber) -> MySubscriber {
    gn.subscribe("SimpleRustDataProduct", &subscriber);
    subscriber
}


fn consume(data: impl GravitySubscriber) {
    let x= data;
    // warn!("All gone!");
}
#[test]
fn outside_function () {

    let mut gnn = GravityNode::new();
    gnn.init("SimpleGravityComponent");

    let mut sub = MySubscriber {};

    sub = external_subscribe(&mut gnn, sub);
    gnn.subscribe("SimpleRustDataProduct", &sub);
    
    consume(sub);
    //testing a dropped subscriber struct. Still works
    {
        let sub = MySubscriber {};
        gnn.subscribe("SimpleRustDataProduct", &sub);
    }

    let data_product_id = "SimpleRustDataProduct";
    let gn = GravityNode::new();
    let mut ret = gn.init("RustExample");
    let fs = gn.get_int_param("Fs", 0);
   
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to initialize GravityNode (return code {:?})", ret);
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
        data.set_multiplicand_b(count + 1024);

    //     //TODO, but that should be all
        gdp.set_data(&data);
        
        
        ret = gn.publish(&gdp);
        if ret != GravityReturnCode::SUCCESS {
            error!("Could not publish data product (return code {:?})", ret);
            std::process::exit(1)
        }
        // if count == 9 { gnn.unsubscribe(data_product_id, &sub); 
        //     std::thread::sleep(time::Duration::from_millis(300));}

        if count == 15 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_millis(100));
    }

    // gn.wait_for_exit();  
}


struct MyDropSubscriber {}

impl GravitySubscriber for MyDropSubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        for i in data_products.iter() {
            let mut pb = MultPB::new();
            i.populate_message(&mut pb);
        //    warn!("got {}, {}", pb.multiplicand_a(), pb.multiplicand_b());
            panic!(); //shhould never get here!
            // assert!(pb.multiplicand_b() == (pb.multiplicand_a() + 1024));
        }
    }
}
#[test]
fn dropped_node() {
    let mut gnn = GravityNode::new();
    gnn.init("SimpleGravityComponent");

    let sub = MyDropSubscriber {};

    gnn.subscribe("SimpleRustDataProduct", &sub);
    {
        let to_drop = gnn;
    }
    

    let data_product_id = "SimpleRustDataProduct";
    let gn = GravityNode::new();
    let mut ret = gn.init("RustExample");
    let fs = gn.get_int_param("Fs", 0);
   
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to initialize GravityNode (return code {:?})", ret);
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
        data.set_multiplicand_b(count + 1024);

    //     //TODO, but that should be all
        gdp.set_data(&data);
        
        
        ret = gn.publish(&gdp);
        if ret != GravityReturnCode::SUCCESS {
            error!("Could not publish data product (return code {:?})", ret);
            std::process::exit(1)
        }
        // if count == 9 { gnn.unsubscribe(data_product_id, &sub); 
        //     std::thread::sleep(time::Duration::from_millis(300));}

        if count == 15 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_millis(100));
    }
    
}