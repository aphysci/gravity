#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_assignments)]
#![allow(dead_code)]
#![allow(unused_imports)]

mod gravity;
mod ffi1;
mod protos;
use std::ffi::c_char;
use std::time;
use autocxx::subclass::{subclass, CppSubclassDefault};
use spdlog::prelude::*;

use crate::{protos::DataPB::*};
use crate::gravity::*;
use crate::protos::ffi::*;
use ffi1::{GravityReturnCode, GravityTransportType};




fn main() {
    
    gravity_logger::info("Beginning rust version of gravity");

    let gn = GravityNode::new();
    let mut ret = gn.init("RustNode");
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to initialize GravityNode (return code {:?})", ret);
        std::process::exit(1);
    }
    info!("Gravity returned code SUCCESS. Init successful");


    let dataProductID = "RustDataProduct";

    ret = gn.register_data_product(&dataProductID, GravityTransportType::TCP);
    if ret != GravityReturnCode::SUCCESS {
        critical!("Unable to register data product (return code {:?})", ret);
        std::process::exit(1)
    }
    


    let mut quit = false;
    let mut has_subs = false;
    let mut count = 1;
    while !quit
    {   
        

        let gdp = GravityDataProduct::new(&dataProductID);

        // let mut data = "HelloRustWorld #".to_owned();
        // data.push_str(&count.to_string());



        let mut data = MultPB::new();
        data.set_multiplicand_a(count);
        data.set_multiplicand_b(count + 4);

        //TODO, but that should be all
        gdp.setData(&data);
    
        ret = gn.publish(gdp);
        if ret != GravityReturnCode::SUCCESS {
            error!("Could not publish data product (return code {:?})", ret);
            std::process::exit(1)
        }

        gn.subscribers_exist(dataProductID, &mut has_subs);
        if has_subs {info!("Has subscribers");} else {warn!("Has no subscriber :(")}
        if count == 20 { quit = true;}
        count += 1;

        std::thread::sleep(time::Duration::from_secs(1));
    }
    gn.wait_for_exit();
   
}
