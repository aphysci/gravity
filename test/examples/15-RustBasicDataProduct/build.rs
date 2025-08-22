use std::path::Path;

// build script to auto generate protobufs
fn main() {

    let dir = std::env::var("CARGO_MANIFEST_DIR").unwrap();
    
    let root = Path::new(&dir);
    let path = root.parent().unwrap();
    let mut path = path.to_str().unwrap().to_string();
    path.push_str("/protobuf");
    let mut file = path.clone();
    file.push_str("/BasicCounterDataProduct.proto");

    protobuf_codegen::Codegen::new()
        // .protoc_path(&protoc_bin_vendored::protoc_bin_path().unwrap())
        // .protoc()
        .pure()
        .include(path)
        .input(file)
        .cargo_out_dir("protobuf")
        .run_from_script();

   
}