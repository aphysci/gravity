#ifndef RUSTGRAVITYREQUESTOR_H_
#define RUSTGRAVITYREQUESTOR_H_


#include <GravityRequestor.h>
#include "rust/cxx.h"

namespace gravity
{
    class RustRequestor : public GravityRequestor {
        private:
            rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> filled;
            rust::Fn<void(const std::string&, const std::string&, size_t)> timeout;
            size_t addr;

            public:
                RustRequestor(rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> filled, rust::Fn<void(const std::string&, const std::string&, size_t)> timeout, size_t addr);
                virtual void requestFilled(std::string serviceID, std::string requestID,
                                            const GravityDataProduct& response);
                virtual void requestTimeout(std::string serviceID, std::string requestID);
        };

    std::unique_ptr<RustRequestor> rustRustRequestor(rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> func, rust::Fn<void(const std::string&, const std::string&, size_t)> timeout, size_t addr);

   
    
    
} // namespace gravity

    
    
#endif