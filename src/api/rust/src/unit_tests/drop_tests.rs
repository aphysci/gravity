use core::time;

use crate::{gravity_data_product::GravityDataProduct, gravity_node::{GravityNode, GravityTransportType}, gravity_subscriber::GravitySubscriber, protos::BasicCounterDataProduct::BasicCounterDataProductPB, spdlog::SpdLog};

struct MySubscriber {}

impl GravitySubscriber for MySubscriber {
    fn subscription_filled(&mut self, data_products: &Vec<GravityDataProduct>) {
        for i in data_products.iter() {
           let mut pb = BasicCounterDataProductPB::new();
           i.populate_message(&mut pb);
        //    warn!("got {}, {}", pb.multiplicand_a(), pb.multiplicand_b());
           SpdLog::warn(format!("Count: {}", pb.count())); 
        }
    }
}

fn mover<T> (x: T) -> T {
    x
}

#[test]
fn subscriber_drop() {
    let handle = std::thread::spawn( || {
        let gn = GravityNode::new();
        gn.init("DropPublisherComponent");

        gn.register_data_product("DropDataProduct", GravityTransportType::TCP);

        let mut count = 1;
        loop {

            let data_product = GravityDataProduct::from_id("DropDataProduct");
            
            let mut data = BasicCounterDataProductPB::new();
            data.set_count(count);

            data_product.set_data(&data);

            gn.publish(&data_product);

            count += 1;
            std::thread::sleep(time::Duration::from_secs(1));
        }
    });

    let mut gn = GravityNode::new();
    gn.init("DropSubscriberComponent");

    let subscriber = MySubscriber {};

    gn.subscribe("DropDataProduct", &subscriber);

    std::thread::sleep(time::Duration::from_secs(2));

    let moved = mover(gn);
    SpdLog::warn("Moved Node");

    std::thread::sleep(time::Duration::from_secs(2));

    let moved_sub = mover(subscriber);
    SpdLog::warn("Moved Subscriber");

    std::thread::sleep(time::Duration::from_secs(2));

    std::mem::drop(moved_sub);

    SpdLog::warn("Dropped subscriber...");

    std::thread::sleep(time::Duration::from_secs(2));


}