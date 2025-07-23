// use std::env;
use cmake;
use std::{env, path::Path};


fn main() {
    //bridge sources
    let src = ["lib/RustSpdLog.cpp", "lib/RustGravityNode.cpp", 
        "lib/RustGravityDataProduct.cpp", "lib/RustFutureResponse.cpp",
         "lib/RustGravitySubscriber.cpp", "lib/RustGravityRequestor.cpp",
         "lib/RustGravityHeartbeatListener.cpp", "lib/RustGravitySubscriptionMonitor.cpp",
         "lib/RustGravityServiceProvider.cpp"
         ];
    let mut srcs = Vec::with_capacity(9);
    for s in src.iter() {
        let mut to_add = "src/api/rust/".to_string();
        to_add.push_str(s);
        srcs.push(to_add);
    }

    // get the necessary library and include paths, relative to the Cargo.toml
    let dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let root = Path::new(&dir);
    let lib_path = root.join("build/install/lib/");
    let include_path = root.join("build/install/include");

    // cmake the gravity libraries
    let out_dir = root.join("build");
    let prefix = root.join("build/install");
    
    let _install_dir = cmake::Config::new(root.as_os_str())
        .define("SKIP_PYTHON", "ON")
        .define("SKIP_JAVA", "ON")
        .define("CMAKE_INSTALL_PREFIX", prefix.as_os_str())
        // .define("GRAVITY_USE_EXTERNAL_PROTOBUF", "TRUE")
        // .define("GRAVITY_USE_EXTERNAL_ZEROMQ", "FALSE")
        .define("BUILD_EXAMPLES_TESTS", "OFF")
        .out_dir(out_dir.as_os_str())
        .build();

    //compile the bridge
    cxx_build::bridge("src/api/rust/src/ffi.rs")
        .files(srcs.iter())
        .include(include_path)
        .compile("rust_gravity");
    
    // search and link the libraries created
    // note the *_d. This is what the cmake crate does, but it should not matter the name
    println!("cargo:rustc-link-search={}", lib_path.display());
    println!("cargo:rustc-link-lib=gravity_protobufs_d");
    println!("cargo:rustc-link-lib=gravity_d");
    println!("cargo:rustc-link-lib=protobuf");
    println!("cargo:rustc-link-lib=zmq");


    // cargo run will add this to the library path. 
    // If you cargo build then run your binary it might not work
    println!("cargo:rustc-env=LD_LIBRARY_PATH={}", lib_path.display()); 
    // println!("cargo:rustc-flags='-L{}'", lib_path.display());
   
}