#include "RustGravityRequestor.h"

namespace gravity
{
     RustRequestor::RustRequestor(
        rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> filled,rust::Fn<void(const std::string&, const std::string&, size_t)> timeout, size_t addr)
    {
        this->filled = filled;
        this->timeout = timeout;
        this->addr = addr;
    }

    std::unique_ptr<RustRequestor> rustRustRequestor(
        rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> filled, rust::Fn<void(const std::string&, const std::string&, size_t)> timeout, size_t addr)
    {
        return std::unique_ptr<RustRequestor>(new RustRequestor(filled, timeout, addr));
    }

    void RustRequestor::requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response)
    {
        filled(serviceID, requestID, response, this->addr);
    }

    void RustRequestor::requestTimeout(std::string serviceID, std::string requestID)
    {
        timeout(serviceID, requestID, addr);
    }
} // namespace gravity
