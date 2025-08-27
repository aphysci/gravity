#ifndef RUSTGRAVITYSERVICEPROVIDER_H_
#define RUSTGRAVITYSERVICEPROVIDER_H_

#include <GravityServiceProvider.h>
#include "rust/cxx.h"

struct ServiceWrap;

namespace gravity
{
    class RustServiceProvider : public GravityServiceProvider {
        private:    
            rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, ServiceWrap *)> func;
            ServiceWrap * servicePtr;
        public:
            RustServiceProvider(rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, ServiceWrap *)> func, ServiceWrap * servicePtr);
            virtual std::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    };

    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, ServiceWrap *)> func, ServiceWrap * servicePtr);

} // namespace gravity


#endif