#include "RustGravitySubscriber.h"

#include "gravity/src/api/rust/src/ffi.rs.h"

namespace gravity
{

    RustSubscriber::RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct>&, const SubscriberWrap*)> func, size_t addr, const SubscriberWrap* box) : box(box)
    {
        this->func = func;
        this->addr = addr;
        // this->wrap = wrap;
        // this->box = box;
        std::cout << "calling in consturvtro\n";
        std::vector<GravityDataProduct> v;
        this->func(v, this->box);
    }

    void RustSubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
    {
        std::vector<GravityDataProduct> v;
        for (std::vector < std::shared_ptr<GravityDataProduct>>::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
        {
            GravityDataProduct to_add = **i;
            v.push_back(to_add);
        }
        std::cout << "calling the func...\n";
        // auto x = this->wrap;
        // std::cout << "acessed m,emmeber\n";
        this->func(v, this->box);
    }

    

    std::unique_ptr<RustSubscriber> newRustSubscriber(
        rust::Fn<void(const std::vector<GravityDataProduct>&, const SubscriberWrap*)> func, size_t addr, const SubscriberWrap* box)
    {
        
        // wrap.box = std::move(box);
        return std::unique_ptr<RustSubscriber>(new RustSubscriber(func, addr, box));
    }

    
    
} // namespace gravity
