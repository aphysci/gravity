
fn main() {
    let src = ["lib/dummy.cpp"];
    // println!("cargo:rustc-link-lib=add");   "libs/mult.cpp",
    cxx_build::bridge("src/ffi1.rs").files(src.iter()).compile("dummy");

    println!("cargo:rerun-if-changed=src/main.rs");
    println!("cargo:rerun-if-changed=src/gravity.rs");
    println!("cargo:rerun-if-changed=src/ffi1.rs");
    println!("cargo:rustc-link-search=lib/");
    println!("cargo:rustc-link-lib=gravity_protobufs");
    println!("cargo:rustc-link-lib=gravity");
    println!("cargo:rustc-link-lib=spdlog");
   
}