#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_assignments)]
#![allow(dead_code)]
#![allow(unused_imports)]

mod gravity;
mod ffi;
mod protos;
use std::ffi::c_char;
use std::time;
use crate::protos::DataPB::*;
use crate::gravity::*;
use crate::ffi::{GravityReturnCode, GravityTransportType};

fn main() {
    
    spdlog::info("Beginning rust version of gravity");

    let gn = Node::new();
    let mut ret = gn.init("RustNode");
    if ret != GravityReturnCode::SUCCESS {
        spdlog::critical("Unable to initialize GravityNode");
        std::process::exit(1);
    }

    spdlog::info("Gravity returned code SUCCESS. Init successful");


    let dataProductID = "RustDataProduct";

    ret = gn.registerDataProduct(&dataProductID, GravityTransportType::TCP);
    if ret != GravityReturnCode::SUCCESS {
        spdlog::critical("Unable to register data product");
        std::process::exit(1)
    }

    let mut quit = false;
    let mut count = 1;
    while !quit
    {   
        std::thread::sleep(time::Duration::from_secs(1));

        let gdp = DataProduct::new(&dataProductID);

        // let mut data = "HelloRustWorld #".to_owned();
        // data.push_str(&count.to_string());


        
        let mut data = MultPB::new();
        data.set_multiplicand_a(count);
        data.set_multiplicand_b(count + 4);

        //TODO, but that should be all
        gdp.setData(&data);
    
        ret = gn.publish(gdp);
        if ret != GravityReturnCode::SUCCESS {
            spdlog::error("Could not publish data product");
            std::process::exit(1)
        }

        if count == 20 { quit = true;}
        count += 1;
    }
}
