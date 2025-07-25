//! 
//! 
//! The goal of this crate is to provide Rust bindings to Gravity 
//! it relies on the crate [CXX](https://crates.io/crates/cxx).
//! 
//! This crate compiles the Gravity C++ library and links it with Rust code to give a 
//! seamless appearance of idiomatic Rust.
//! 
//!  <br>
//! 
//! 
//!  Dependencies for using this crate are cmake, a C++ compiler (e.g. g++), bison, flex 
//!  Make sure these 4 items are installed before adding this crate as a dependency.
//! ```sh
//!     sudo apt install cmake g++ bison flex
//! ``` 
//! 
//! *Compiler support: requires rustc 1.73+ and c++11 or newer*<br>
//! 
//!    <br>
//! # Overview
//!
//! The idea is to call directly into the libraries compiled by [Gravity](https://github.com/aphysci/gravity)
//! The CXX crate does not support non-const member functions, so in order to make this feel like idiomatic rust
//! each struct is a wrapper that contains a pointer to a C++ object (i.e. GravityNode or GravityDataProduct) and 
//! and its member functions supply that as pointer as a paramter to the C++ function call. 
//! 
//! To make this work, there must be a bridge on each side of the language barrier. These two items work together to 
//! provide a transition from Rust to C++ that feels like Rust without modifying the original Gravity source code.
//! 
//! The way that this crate deals with the concept of inheritance is by giving the work to C++.
//! In the C++ files, there are classes that inherit from the abstract classes from Gravity in C++ (i.e. RustSubscriber 
//! inherits from GravitySubscriber). These Rust classes take a function pointer to a rust function that converts C++ types 
//! to Rust types, and a pointer to the trait object that Rust treats as a subscriber. This will make more sense in the subscriber example.
//! 
//! The function pointer unwraps the pointer into its trait object, and calls the method with the "rustified" types.
//! 
//! # Example: Basic Gravity Publisher/Subscriber
//!
//! ## Publisher
//! ```rust
//! fn main () {
//!   
//!    let gn = GravityNode::new();
//!    gn.init("RustProtobufExample");
//!  
//!    gn.register_data_product(
//!        //identifies the data product to the service directory so others 
//!        //can subscribe to it
//!        "BasicCounterDataProduct",
//!        //Assign a transport type to the socket (almost always tcp, unless you are only
//!        //using the gravity data product between two processes on the same computer)
//!         GravityTransportType::TCP);    
//!
//!    //TODO: set this when you want the program to quit if you need to 
//!    //clean up before exiting
//!    let mut quit = false; 
//!    let mut count = 1;
//!    while !quit
//!    {   
//!        //create a data product to send across the network of type "BasicCounterDataProduct"
//!        let counter_data_product = GravityDataProduct::from_id("BasicCounterDataProduct");
//!
//!        //Initialize our message
//!        let mut data = BasicCounterDataProductPB::new();
//!        data.set_count(count);
//!        
//!        //Put message into data product
//!        counter_data_product.set_data(&data);
//!        
//!        //Publish the data product
//!        gn.publish(&gdp);
//!
//!        //Increment count
//!        count += 1;
//!        if count > 50 {
//!            count = 1;
//!        }
//!
//!        //Sleep for 1 second.
//!        std::thread::sleep(time::Duration::from_secs(1));
//!    }
//!
//!    gn.wait_for_exit();  
//!}
//!
//!```
//!  
//! ## Subscriber
//! 
//! ```rust
//!    struct MySubscriber {}
//!
//!    impl GravitySubscriber for MySubscriber {
//!        
//!        fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
//!            
//!            for data_product in data_products.iter() {
//!                //Get the protobuf object from the message
//!                let mut counter_data_pb = BasicCounterDataProductPB::new();
//!                data_product.populate_message(&mut counter_data_pb)
//!
//!                //Process the message
//!                warn!("Current Count: {}", counter_data_pb.count());
//!            
//!
//!    
//!            }
//!        }
//!    }
//!
//!```
//!
//!And then to use the subscriber:
//!
//!```rust
//!    let mut gn = GravityNode::new();
//!    gn.init("ProtobufDataProductSubscriber");
//!
//!    //This is just an example, you can have any other way to 
//!    //instantiate your own GravitySubscriber, as long as it impl GravitySubscriber trait
//!    let subscriber = MySubscriber {}
//!
//!    //subscribe to the data product
//!    //this function takes in any struct that impl GravitySubscriber
//!    gn.subscribe("BasicCounterDataProduct", &subscriber);
//!```
//! Note that if a GravityNode is to subscribe, it must be declared as mut
//! since GravityNodes hold a reference to the subscriber object it creates.
//! 
//! # Cargo based setup
//! 
//! Assuming you have the necessary dependencies installed, all you need is to add
//! Gravity to your Cargo.toml
//! ```toml
//! # Cargo.toml
//! 
//! [dependencies]
//! gravity = { git = "https://github.com/astrauc/gravity.git" }
//! ```
//! 
//! 




#![allow(dead_code)]

mod spdlog;
mod gravity_node;
mod gravity_data_product;
mod gravity_subscriber;
mod gravity_requestor;
mod gravity_service_provider;
mod future_response;
mod gravity_heartbeat_listener;
mod gravity_subscription_monitor;
mod protos;
mod ffi;
mod unit_tests;

pub use crate::gravity_data_product::GravityDataProduct;
pub use crate::gravity_node::{GravityNode, GravityReturnCode, GravityTransportType};
pub use crate::gravity_requestor::GravityRequestor;
pub use crate::gravity_service_provider::GravityServiceProvider;
pub use crate::gravity_subscriber::GravitySubscriber;
pub use crate::gravity_heartbeat_listener::GravityHeartbeatListener;
pub use crate::gravity_subscription_monitor::GravitySubscriptionMonitor;
pub use crate::future_response::FutureResponse;
pub use crate::spdlog::SpdLog;

