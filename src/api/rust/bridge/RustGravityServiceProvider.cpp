#include "RustGravityServiceProvider.h"

namespace gravity
{
    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func,
        size_t addr)
    {
        return std::unique_ptr<RustServiceProvider>(new RustServiceProvider(func, addr));
    } 
    
    RustServiceProvider::RustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func,
        size_t addr)
    {
        this->func = func;
        this->addr = addr;
    }

    std::shared_ptr<GravityDataProduct> RustServiceProvider::request(const std::string serviceID,
                                                                     const GravityDataProduct& dataProduct)
    {
        return this->func(serviceID, dataProduct, this->addr);
    }
} // namespace gravity
