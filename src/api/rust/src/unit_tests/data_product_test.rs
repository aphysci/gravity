include!(concat!(env!("OUT_DIR"), "/protobuf/mod.rs"));

use crate::GravityDataProduct;
use DataPB::*;


#[test]
fn data_product_functions() {
    let gdp = GravityDataProduct::with_id("TestDataProduct");

    let mut pb = MultPB::new();
    pb.set_multiplicand_a(5);
    pb.set_multiplicand_b(9);

    gdp.set_data(&pb);

    assert!(gdp.size() > 0);
    assert!(gdp.data_size() > 0);
    assert!(gdp.data_product_id() == String::from("TestDataProduct"));

    gdp.set_is_cached_data_product(true);
    assert!(gdp.is_cached_data_product());
    gdp.set_is_cached_data_product(false);
    assert!(!gdp.is_cached_data_product());

    gdp.set_is_relayed_data_product(true);
    assert!(gdp.is_relayed_data_product());
    gdp.set_is_relayed_data_product(false);
    assert!(!gdp.is_relayed_data_product());

    gdp.set_component_id("newCompId");
    assert_eq!("newCompId", gdp.component_id());

    gdp.set_timestamp(18);
    
    gdp.set_software_version("some version");
    assert_eq!("some version".to_string(), gdp.software_version());

    gdp.set_type_name("multiply");
    assert_eq!("multiply", gdp.type_name());

    let array = gdp.serialize_to_array();
    let gdp2 = GravityDataProduct::with_id("ParseSerializeTest");

    gdp2.parse_from_array(&array);
    assert_eq!(gdp.size(), gdp2.size());
    assert_eq!(gdp.data_size(), gdp2.data_size());
    let mut pb2 = MultPB::new();
    gdp2.populate_message(&mut pb2);
    assert_eq!(5, pb2.multiplicand_a());
    assert_eq!(9, pb2.multiplicand_b());



}