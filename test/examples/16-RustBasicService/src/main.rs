include!(concat!(env!("OUT_DIR"), "/protobuf/mod.rs"));

use core::time;
use std::sync::Mutex;

use gravity::{GravityDataProduct, GravityNode, GravityRequestor, GravityReturnCode, GravityServiceProvider, GravityTransportType, SpdLog};
use Multiplication::*;

static GOT_ASYNC: Mutex<bool> = Mutex::new(false);
struct MultiplicationServiceProvider {}

impl GravityServiceProvider for MultiplicationServiceProvider {
    fn request(&mut self, _service_id: &str, data_product: &GravityDataProduct) -> GravityDataProduct {
        
        if data_product.data_product_id() != "Multiplication" {
            SpdLog::error("Request is not for multiplication!");
            return GravityDataProduct::with_id("BadRequest");
        }

        // Get the params for this result
        let mut params = MultiplicationOperandsPB::new();
        data_product.populate_message(&mut params);

        SpdLog::warn(format!(
            "Request Received: {} x {}",
            params.multiplicand_a(),
            params.multiplicand_b()));

        // Do the calculation
        let result = params.multiplicand_a() * params.multiplicand_b();

        // Return the results to the requestor
        let mut result_pb = MultiplicationResultPB::new();
        result_pb.set_result(result);

        let result_gdp = GravityDataProduct::with_id("MultiplicationResult");
        result_gdp.set_data(&result_pb);

        result_gdp
    }   
}

struct MultiplicationRequestor {}

impl GravityRequestor for MultiplicationRequestor {
    fn request_filled(&mut self, _service_id: &str, request_id: &str, response: &GravityDataProduct) {
        // Parse the message into a protobuf
        let mut result = MultiplicationResultPB::new();
        response.populate_message(&mut result);

        // Write the answer
        SpdLog::warn(format!(
            "Asynchronous response received: {} = {}", request_id, result.result()
        ));
        
        let mut data = GOT_ASYNC.lock().expect("Something already has this");
        *data = true;
        
        
        
        

        
    }

    fn request_timeout(&mut self, _: &str, _: &str) {
        
    }
}

fn main() {
    let mut service_gn = GravityNode::new();
    service_gn.init("MultiplicationComponent");

    let msp = service_gn.tokenize_service(MultiplicationServiceProvider {});
    service_gn.register_service(
        // Identifies the service to the service directory so others can make a request to it
        "Multiplication",
        // Almost always going to be TCP
        GravityTransportType::TCP,
        // Give the token for the service provider to register the service.
        &msp);

    // Setup the requests
    let mut gn = GravityNode::new();
    gn.init("MultiplicationRequestor");

    // Set up the first multiplication request
    let requestor = gn.tokenize_requestor(MultiplicationRequestor {});
    let mult_request1 = GravityDataProduct::with_id("Multiplication");
    let mut params1 = MultiplicationOperandsPB::new();
    params1.set_multiplicand_a(8);
    params1.set_multiplicand_b(2);
    mult_request1.set_data(&params1);

    while gn.request_async_with_request_id(
        "Multiplication",  // Service name
        &mult_request1,       // Request
        &requestor,                   // token representing the object with the callback
        "8 x 2"           // string identifying which request this is
    ) != GravityReturnCode::SUCCESS {
        SpdLog::warn("request to Multiplication servicec failed, retrying...");
        std::thread::sleep(time::Duration::from_millis(1000));
    }

    // Set up the second multiplication request
    let mult_request2 = GravityDataProduct::with_id("Multiplication");
    let mut params2 = MultiplicationOperandsPB::new();
    params2.set_multiplicand_a(5);
    params2.set_multiplicand_b(7);
    mult_request2.set_data(&params2);

    // Make a synchronous request for multiplication
    let mult_sync = gn.request_sync_with_timeout(
        "Multiplication",   // Service name
        &mult_request2,        // Request
        1000);    // Timeout in milliseconds

    match mult_sync {
        None => SpdLog::error("Request returned None"),
        Some( gdp ) => {
            let mut result = MultiplicationResultPB::new();
            gdp.populate_message(&mut result);

            SpdLog::warn(format!(
                "Synchronous response received: 5 x 7 = {}", result.result()
            ));

        }
    }
    
    while {
        !*GOT_ASYNC.lock().expect("lock held")
     } {
        SpdLog::warn("Waiting for asyn request");
        std::thread::sleep(time::Duration::from_millis(1000));
    }

}
