#ifndef RUSTGRAVITYREQUESTOR_H_
#define RUSTGRAVITYREQUESTOR_H_


#include <GravityRequestor.h>
#include "rust/cxx.h"

struct RequestorWrap;

namespace gravity
{
    class RustRequestor : public GravityRequestor {
        private:
            rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, RequestorWrap *)> filled;
            rust::Fn<void(const std::string&, const std::string&, RequestorWrap *)> timeout;
            RequestorWrap * requestorPtr;

            public:
                RustRequestor(rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, RequestorWrap *)> filled,
                 rust::Fn<void(const std::string&, const std::string&, RequestorWrap *)> timeout,
                 RequestorWrap * requestorPtr);
                virtual void requestFilled(std::string serviceID, std::string requestID,
                                            const GravityDataProduct& response);
                virtual void requestTimeout(std::string serviceID, std::string requestID);
        };

    std::unique_ptr<RustRequestor> rustRustRequestor(rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, RequestorWrap *)> func, rust::Fn<void(const std::string&, const std::string&, RequestorWrap *)> timeout, RequestorWrap * requestorPtr);

   
    
    
} // namespace gravity

    
    
#endif