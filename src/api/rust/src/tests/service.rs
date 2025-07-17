use core::time;
// pub mod protos;

use crate::ffi::new_future_response;
use crate::spdlog::SpdLog;
use crate::gravity_service_provider::GravityServiceProvider;
use crate::gravity_data_product::GravityDataProduct;
use crate::gravity_requestor::GravityRequestor;
use crate::gravity_node::{GravityNode, GravityReturnCode, GravityTransportType};
// use gravity::gravity_logger::SpdLog;
use crate::protos::DataPB::{MultPB, ResultPB};
use crate::protos::BigComplexPB::{self, BigGuyPB, BigResultPB, SmallGuyPB};

struct MyProvider {}

impl GravityServiceProvider for MyProvider {
    fn request(&mut self, service_id: &str, data_product: &GravityDataProduct) -> GravityDataProduct {
        if data_product.get_data_product_id() != String::from("Multiplication") {
            SpdLog::error("Request is not for multiplication");
        }
        let mut mult_ops = MultPB::new();
        data_product.populate_message(&mut mult_ops);

        // SpdLog::warn(format!("Request recieved {} x {}", mult_ops.multiplicand_a.unwrap(), mult_ops.multiplicand_b.unwrap()));
        

        let result = mult_ops.multiplicand_a.unwrap() * mult_ops.multiplicand_b.unwrap();

        let mut result_pb = ResultPB::new();
        result_pb.set_result(result);

        let ret = GravityDataProduct::from_id("MultiplicationResult");
        ret.set_data(&result_pb);
        ret
    }
}
struct MyRequestor {}

impl GravityRequestor for MyRequestor {
    fn request_filled(&mut self, _service_id: &str, request_id: &str, response: &GravityDataProduct) {
        let mut result_pb = ResultPB::new();
        response.populate_message(&mut result_pb);

        assert_eq!(result_pb.result(), 16);
   
    }
    
    fn request_timeout(&mut self, service_id: &str, request_id: &str) {
        assert!(true);
    }
}

#[test]
fn service() {

    let mut gn = GravityNode::new();

    gn.init("MultiplicationComponent");

   
    let msp = MyProvider {};
    gn.register_service("Multiplication",
     GravityTransportType::TCP, &msp);

     let mut gn2 = GravityNode::new();

     let mut ret = gn2.init("MultiplicationRequestor");

     while ret != GravityReturnCode::SUCCESS {
        SpdLog::warn("Unable to init component, retrying...");
        ret = gn2.init("MultiplicationRequestor");
     }

    let requestor = MyRequestor {};

    let mult_request = GravityDataProduct::from_id("Multiplication");
    let mut operands = MultPB::new();

    operands.set_multiplicand_a(8);
    operands.set_multiplicand_b(2);
    mult_request.set_data(&operands);
    
    ret = gn2.request_async("Multiplication", &mult_request, &requestor, Some("8 x 2"), None, Some(""));
    while ret != GravityReturnCode::SUCCESS {
        SpdLog::warn("request to Multiplication service failed, retrying...");
        std::thread::sleep(time::Duration::from_secs(1));

        ret = gn2.request_async("Multiplication", &mult_request, &requestor, Some("8 x 2"), None, None as Option<&str>);
    }

    let request2 = GravityDataProduct::from_id("Multiplication");
    let mut operands2 =  MultPB::new();
    operands2.set_multiplicand_a(5);
    operands2.set_multiplicand_b(7);


    request2.set_data(&operands2);
    let mult_sync = gn2.request_sync("Multiplication", &request2);

    match mult_sync {
        None => panic!(),
        Some(gdp) => {
            let mut result2 = ResultPB::new();
            gdp.populate_message(&mut result2);
            assert_eq!(35, result2.result());
            // SpdLog::warn(format!("Synchronous response recieved: 5 x 7 = {}", result2.result()));
        }
    }
}
struct BetterProvider {}

impl GravityServiceProvider for BetterProvider {
    fn request(&mut self, service_id: &str, data_product: &GravityDataProduct) -> GravityDataProduct {
        let mut bigops = BigGuyPB::new();
        data_product.populate_message(&mut bigops);
        // SpdLog::warn(format!("Request recieved for Big Complex"));
        
        
        let mut bigresult = BigResultPB::new();
        bigresult.set_didIt(bigops.shouldI());
        bigresult.set_length(bigops.littles.len() as i32);
        assert_eq!(true, bigops.has_helloworld());
        assert_eq!("HelloWorld", bigops.helloworld());
        assert_eq!(true, bigops.has_bigNumber());
        assert_eq!(1785, bigops.bigNumber());
        
        for small in bigops.littles.iter() {
            bigresult.littleNums.push(small.number());
            bigresult.proverbs.push(small.proverb().to_string());
            for val in small.values.iter() {
                bigresult.values.push(*val);
            }
        }

        let result_gdp = GravityDataProduct::from_id("BigComplex");
        result_gdp.set_data(&bigresult);
        
        result_gdp

    }
}

struct BetterRequestor {}

impl GravityRequestor for BetterRequestor {
    fn request_filled(&mut self, service_id: &str, request_id: &str, response: &GravityDataProduct) {
        let mut bigresult = BigResultPB::new();
        response.populate_message(&mut bigresult);

        assert_eq!(true, bigresult.didIt());
        assert_eq!(3, bigresult.length());
        let mut count = 0;
        for num in bigresult.littleNums.iter() {
            assert_eq!(count, *num);
            count += 1;
        }
        count = 0;
        for val in bigresult.values.iter() {
            assert_eq!(count, *val);
            count += 2;
        }
        assert_eq!(3,  bigresult.proverbs.len());
        for (index, proverb) in bigresult.proverbs.iter().enumerate() {
            
            match index {
                0 => assert_eq!("I love unit tests", proverb),
                1 => assert_eq!("I hate unit tests", proverb),
                2 => assert_eq!("What's a unit test?", proverb),
                _ => panic!(),
            }
            
        }
    }
    
    fn request_timeout(&mut self, service_id: &str, request_id: &str) {
        assert!(true)
    }
}

#[test]
fn service2 () {
    let mut gn = GravityNode::new();
    gn.init("BigComplexComponentq");

    let msp = BetterProvider {};
    gn.register_service("BigComplex",
     GravityTransportType::TCP, &msp);

     let mut gn2 = GravityNode::new();

     let mut ret = gn2.init("BigComplexRequestor");

     while ret != GravityReturnCode::SUCCESS {
        SpdLog::warn("Unable to init component, retrying...");
        ret = gn2.init("BigComplexRequestor");
     }

    let requestor = BetterRequestor {};

    let mult_request = GravityDataProduct::from_id("BigComplex");
    let mut operands = BigGuyPB::new();

    operands.set_bigNumber(1785);
    operands.set_helloworld("HelloWorld".to_string());
    operands.set_shouldI(true);
    for _ in 0..3 {
        let to_add = SmallGuyPB::new();
        operands.littles.push(to_add);
    }

    let mut count = 0;
    let list = ["I love unit tests", "I hate unit tests", "What's a unit test?"];
    for (index , small) in operands.littles.iter_mut().enumerate() {
        small.set_number(index as i32);
        small.set_proverb(list.get(index).unwrap().to_string());
        for _ in 0..3 {
            small.values.push(count);
            count += 2;
        }
        
    }
    mult_request.set_data(&operands);
    
    ret = gn2.request_async("BigComplex", &mult_request, &requestor, Some(""), None, Some(""));
    while ret != GravityReturnCode::SUCCESS {
        SpdLog::warn("request to Multiplication service failed, retrying...");
        std::thread::sleep(time::Duration::from_secs(1));

        ret = gn2.request_async("BigComplex", &mult_request, &requestor, Some(""), None, None as Option<&str>);
    }

    let request2 = GravityDataProduct::from_id("BigComplex");
    let mut operands2 =  BigGuyPB::new();
    operands2.set_bigNumber(1785);
    operands2.set_helloworld("HelloWorld".to_string());
    operands2.set_shouldI(true);
    for _ in 0..3 {
        let to_add = SmallGuyPB::new();
        operands2.littles.push(to_add);
    }

    let mut count = 0;
    let list = ["I love unit tests", "I hate unit tests", "What's a unit test?"];
    for (index , small) in operands2.littles.iter_mut().enumerate() {
        small.set_number(index as i32);
        small.set_proverb(list.get(index).unwrap().to_string());
        for _ in 0..3 {
            small.values.push(count);
            count += 2;
        }
        
    }
    request2.set_data(&operands2);
    let mult_sync = gn2.request_sync("BigComplex", &request2);

    match mult_sync {
        None => panic!(),
        Some(gdp) => {
            let mut result2 = BigResultPB::new();
            gdp.populate_message(&mut result2);
            assert_eq!(true, result2.didIt());
            assert_eq!(3, result2.length());
            let mut count = 0;
            for num in result2.littleNums.iter() {
                assert_eq!(count, *num);
                count += 1;
            }
            count = 0;
            for val in result2.values.iter() {
                assert_eq!(count, *val);
                count += 2;
            }
            assert_eq!(3,  result2.proverbs.len());
            for (index, proverb) in result2.proverbs.iter().enumerate() {
                
                match index {
                    0 => assert_eq!("I love unit tests", proverb),
                    1 => assert_eq!("I hate unit tests", proverb),
                    2 => assert_eq!("What's a unit test?", proverb),
                    _ => panic!(),
                }

            }
        }
    }

    std::thread::sleep(time::Duration::from_secs(1));


}