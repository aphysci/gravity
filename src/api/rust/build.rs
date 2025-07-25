// use std::env;
use cmake;
use std::{env, path::{Path, PathBuf}, str::FromStr};


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
    let mut lib_path = root.to_str().unwrap().to_string();
    lib_path.push_str("/install/lib");
    let mut include_path = root.to_str().unwrap().to_string(); 
    include_path.push_str("install/include");
    let path = PathBuf::from_str("/usr/lib/x86_64-linux-gnu/").unwrap();

    // cmake the gravity libraries
    let out_dir = env::var("OUT_DIR").unwrap();
    let mut prefix = out_dir.clone();
    prefix.push_str("/install");
    
    let _install_dir = cmake::Config::new(root.as_os_str())
        .define("SKIP_PYTHON", "ON")
        .define("SKIP_JAVA", "ON")
        .define("CMAKE_INSTALL_PREFIX", prefix)
        .define("GRAVITY_USE_EXTERNAL_PROTOBUF", "ON")
        .define("GRAVITY_USE_EXTERNAL_ZEROMQ", "ON")
        .define("BUILD_LIBRARY_ONLY", "ON")
        .define("BUILD_EXAMPLES_TESTS", "OFF")
        .out_dir(out_dir)
        .build();

    //compile the bridge
    cxx_build::bridge("src/api/rust/src/ffi.rs")
        .files(srcs.iter())
        .include(include_path)
        .compile("rust_gravity");
    
    // search and link the libraries created
    // note the *_d. This is what the cmake crate does, but it should not matter the name
    println!("cargo:rustc-link-search={}", lib_path);
    println!("cargo:rustc-link-search={}", path.display());
    println!("cargo:rustc-link-lib=static=gravity_d");
    println!("cargo:rustc-link-lib=static=gravity_protobufs_d");
    println!("cargo:rustc-link-lib=static=keyvalue_parser_d");
    println!("cargo:rustc-link-lib=static=protobuf");
    println!("cargo:rustc-link-lib=static=zmq");
    
    // If you want to run the tests...
    // run: sudo apt install libfmt-dev
    // uncomment out the following line
    // println!("cargo:rustc-link-lib=fmt");


    
    // cargo run will add this to the library path. 
    // Only useful for running tests
    // Does nothing otherwise...
    println!("cargo:rustc-env=LD_LIBRARY_PATH={}", lib_path); 
   
}