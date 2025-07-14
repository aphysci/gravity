#include "RustGravitySubscriber.h"

namespace gravity
{
    RustSubscriber::RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct>&, size_t addr)> func, size_t addr) 
    {
        this->func = func;
        this->addr = addr;
    }

    void RustSubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
    {
        std::vector<GravityDataProduct> v;
        for (std::vector < std::shared_ptr<GravityDataProduct>>::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
        {
            GravityDataProduct to_add = **i;
            v.push_back(to_add);
        }
        this->func(v, this->addr);
    }

    

    std::unique_ptr<RustSubscriber> newRustSubscriber(
        rust::Fn<void(const std::vector<GravityDataProduct>&, size_t)> func, size_t addr)
    {
        return std::unique_ptr<RustSubscriber>(new RustSubscriber(func, addr));
    }

    
} // namespace gravity
