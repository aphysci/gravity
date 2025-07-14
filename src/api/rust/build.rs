// use std::env;


fn main() {
    let src = ["lib/RustSpdLog.cpp", "lib/RustGravityNode.cpp", 
        "lib/RustGravityDataProduct.cpp", "lib/RustFutureResponse.cpp",
         "lib/RustGravitySubscriber.cpp", "lib/RustGravityRequestor.cpp",
         "lib/RustGravityHeartbeatListener.cpp", "lib/RustGravitySubscriptionMonitor.cpp",
         "lib/RustGravityServiceProvider.cpp"
         ];
    // println!("cargo:rustc-link-lib=add");   "libs/mult.cpp",
    cxx_build::bridge("src/ffi.rs").files(src.iter()).compile("rust_gravity");

    // let library_path = env::var("LD_LIBRARY_PATH").unwrap();s

    println!("cargo:rerun-if-changed=src/main.rs");
    println!("cargo:rerun-if-changed=src/gravity.rs");
    println!("cargo:rerun-if-changed=src/ffi.rs");
    println!("cargo:rerun-if-changed=lib/shims.h");
    println!("cargo::rustc-link-search=/home/anson/git/gravity/build/install/lib/");
    println!("cargo:rustc-link-lib=gravity_protobufs");
    println!("cargo:rustc-link-lib=gravity");
    println!("cargo:rustc-link-lib=spdlog");
   
}