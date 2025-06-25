use core::{ffi::c_void, time};
use std::{ffi::c_char, ops::Sub};
use protobuf::Message;

// pub use ffi::*;
// use cxx::{let_cxx_string, CxxString, CxxVector, UniquePtr};
// use autocxx::{prelude::*, subclass::subclass};

// use crate::{ffi1::GravityDataProduct, gravity::GraDataProduct};

// include_cpp! {
//     #include "RustSubscriber.h"
//     safety!(unsafe)
//     // extern_cpp_type!("GravityDataProduct", GravityDataProduct)
// }

// #[subclass(superclass("gravity::RustSubscriber"))]
// pub struct GSubscriber {}

// impl ffi::gravity::RustSubscriber_methods for GSubscriber {
//     fn subscriptionFilled(&mut self, dataProducts: &CxxVector<GravityDataProduct>) {
//         println!("HelloWorld")
//     }
// }