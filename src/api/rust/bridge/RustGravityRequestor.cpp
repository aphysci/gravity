#include "RustGravityRequestor.h"
#include "gravity/src/api/rust/src/ffi.rs.h"

namespace gravity
{
     RustRequestor::RustRequestor(
        rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, RequestorWrap *)> filled,
        rust::Fn<void(const std::string&, const std::string&, RequestorWrap *)> timeout,
        RequestorWrap * requestorPtr) : requestorPtr(requestorPtr)
    {
        this->filled = filled;
        this->timeout = timeout;
    }

    std::unique_ptr<RustRequestor> rustRustRequestor(
        rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, RequestorWrap *)> filled,
        rust::Fn<void(const std::string&, const std::string&, RequestorWrap *)> timeout,
        RequestorWrap * requestorPtr)
    {
        return std::unique_ptr<RustRequestor>(new RustRequestor(filled, timeout, requestorPtr));
    }

    void RustRequestor::requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response)
    {
        filled(serviceID, requestID, response, this->requestorPtr);
    }

    void RustRequestor::requestTimeout(std::string serviceID, std::string requestID)
    {
        timeout(serviceID, requestID, this->requestorPtr);
    }
} // namespace gravity
