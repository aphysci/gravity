// use std::env;

use std::{env, path::Path};


fn main() {
    let src = ["lib/RustSpdLog.cpp", "lib/RustGravityNode.cpp", 
        "lib/RustGravityDataProduct.cpp", "lib/RustFutureResponse.cpp",
         "lib/RustGravitySubscriber.cpp", "lib/RustGravityRequestor.cpp",
         "lib/RustGravityHeartbeatListener.cpp", "lib/RustGravitySubscriptionMonitor.cpp",
         "lib/RustGravityServiceProvider.cpp"
         ];
    // println!("cargo:rustc-link-lib=add");   "libs/mult.cpp",
    cxx_build::bridge("src/ffi.rs").files(src.iter()).compile("rust_gravity");

    let dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let cwd = Path::new(&dir);
    let parent = cwd.parent().unwrap().parent().unwrap().parent().unwrap().
        join("build/install/lib");
    // let library_path = env::var("LD_LIBRARY_PATH").unwrap();s
    
    println!("cargo::rustc-link-search={}", parent.display());
    println!("cargo:rustc-link-lib=gravity_protobufs");
    println!("cargo:rustc-link-lib=gravity");
    println!("cargo:rustc-link-lib=spdlog");
   
}