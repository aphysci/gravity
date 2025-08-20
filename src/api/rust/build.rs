// use std::env;
use cmake;
use std::{env, path::{Path, PathBuf}, str::FromStr};

fn main() {
    //bridge sources
    let src = ["bridge/RustSpdLog.cpp", "bridge/RustGravityNode.cpp", 
        "bridge/RustGravityDataProduct.cpp", "bridge/RustFutureResponse.cpp",
         "bridge/RustGravitySubscriber.cpp", "bridge/RustGravityRequestor.cpp",
         "bridge/RustGravityHeartbeatListener.cpp", "bridge/RustGravitySubscriptionMonitor.cpp",
         "bridge/RustGravityServiceProvider.cpp"
         ];
    let mut srcs = Vec::with_capacity(9);
    for s in src.iter() {
        let mut to_add = "src/api/rust/".to_string();
        to_add.push_str(s);
        srcs.push(to_add);
    }

    // get the necessary library and include paths, relative to the Cargo.toml
    let dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let out_dir = env::var("OUT_DIR").unwrap();
    let root = Path::new(&dir);
    let mut lib_path = out_dir.clone();
    lib_path.push_str("/install/lib");
    let mut include_path = out_dir.clone(); 
    include_path.push_str("/install/include");
    let header_path = root.parent().unwrap();
    let _path = PathBuf::from_str("/usr/lib/x86_64-linux-gnu/").unwrap();

    // cmake the gravity libraries
    let mut prefix = out_dir.clone();
    prefix.push_str("/install");
    
    let _install_dir = cmake::Config::new(root.as_os_str())
        .define("SKIP_PYTHON", "ON")
        .define("SKIP_JAVA", "ON")
        .define("CMAKE_INSTALL_PREFIX", prefix)
        .define("GRAVITY_USE_EXTERNAL_PROTOBUF", "ON")
        .define("GRAVITY_USE_EXTERNAL_ZEROMQ", "ON")
        .define("BUILD_LIBRARY_ONLY", "ON")
        .define("BUILD_STATIC_LIBRARIES", "ON")
        .define("BUILD_EXAMPLES_TESTS", "OFF")
        .define("CMAKE_BUILD_TYPE", "Release")
        .cflag("-Wall")
        .cxxflag("-Wall")
        .out_dir(out_dir)
        .build();

    //compile the bridge
    cxx_build::bridge("src/api/rust/src/ffi.rs")
        .files(srcs.iter())
        .include(include_path)
        .include(header_path)
        .warnings(false)
        .cargo_warnings(false)
        .compile("rust_gravity");

    // generate the protobufs for the unit tests
    let mut proto_include = dir.clone();
    proto_include.push_str("/src/api/rust/src/protobuf/");

    let filenames = ["BasicCounterDataProduct.proto",
        "BigComplexPB.proto", "DataPB.proto"];
    let mut proto_files = Vec::new();

    for item in filenames {
        let mut to_add = proto_include.clone();
        to_add.push_str(item);
        proto_files.push(to_add);
    }    

    protobuf_codegen::Codegen::new()
        .pure()
        .include(proto_include)
        .inputs(proto_files)
        .cargo_out_dir("protobuf")
        .run_from_script();


    // search and link the libraries created
    println!("cargo:rustc-link-search={lib_path}");
    // println!("cargo:rustc-link-search={}", path.display());
    println!("cargo:rustc-link-lib=static=gravity");
    println!("cargo:rustc-link-lib=static=gravity_protobufs");
    println!("cargo:rustc-link-lib=static=keyvalue_parser");
    println!("cargo:rustc-link-lib=static=protobuf");
    println!("cargo:rustc-link-lib=static=zmq");
  
   
}