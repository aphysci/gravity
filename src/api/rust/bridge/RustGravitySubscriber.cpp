#include "RustGravitySubscriber.h"

#include "gravity/src/api/rust/src/ffi.rs.h"

namespace gravity
{

    RustSubscriber::RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct>&, SubscriberWrap*)> func, SubscriberWrap* subscriber_ptr) : subscriber_ptr(subscriber_ptr)
    {
        this->func = func;

    }

    void RustSubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
    {
        std::vector<GravityDataProduct> v;
        for (std::vector < std::shared_ptr<GravityDataProduct>>::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
        {
            GravityDataProduct to_add = **i;
            v.push_back(to_add);
        }
        this->func(v, this->subscriber_ptr);
    }

    

    std::unique_ptr<RustSubscriber> newRustSubscriber(
        rust::Fn<void(const std::vector<GravityDataProduct>&, SubscriberWrap*)> func, SubscriberWrap* subscriber_ptr)
    {
        
        // wrap.subscriber_ptr = std::move(subscriber_ptr);
        return std::unique_ptr<RustSubscriber>(new RustSubscriber(func, subscriber_ptr));
    }

    
    
} // namespace gravity
