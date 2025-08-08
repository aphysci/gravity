use std::path::Path;


fn main() {

    let dir = std::env::var("CARGO_MANIFEST_DIR").unwrap();
    let root = Path::new(&dir);
    let path = root.parent().unwrap();
    let mut path = path.to_str().unwrap().to_string();
    path.push_str("/protobuf");


    protobuf_codegen::Codegen::new()
        .pure()
        .protoc()
        .include(path)
        .input("BasicCounterDataProduct.proto");
}