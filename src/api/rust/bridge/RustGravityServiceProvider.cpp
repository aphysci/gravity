#include "RustGravityServiceProvider.h"
#include "gravity/src/api/rust/src/ffi.rs.h"


namespace gravity
{
    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, ServiceWrap *)> func,
        ServiceWrap * servicePtr)
    {
        return std::unique_ptr<RustServiceProvider>(new RustServiceProvider(func, servicePtr));
    } 
    
    RustServiceProvider::RustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, ServiceWrap *)> func,
        ServiceWrap * servicePtr) : servicePtr(servicePtr)
    {
        this->func = func;
    }

    std::shared_ptr<GravityDataProduct> RustServiceProvider::request(const std::string serviceID,
                                                                     const GravityDataProduct& dataProduct)
    {
        return this->func(serviceID, dataProduct, this->servicePtr);
    }
} // namespace gravity
