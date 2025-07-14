#include "RustGravitySubscriptionMonitor.h"

namespace gravity
{
    void RustSubscriptionMonitor::subscriptionTimeout(std::string dataProductID, int millisecondsSinceLast,
                                                 std::string filter, std::string domain)
    {
        this->func(dataProductID, millisecondsSinceLast, filter, domain, this->addr);
    }

    RustSubscriptionMonitor::RustSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, size_t)> func, size_t addr)
    {
        this->func = func;
        this-> addr = addr;
    }
    
    std::unique_ptr<RustSubscriptionMonitor> rustNewSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, size_t)> func, size_t addr)
    {
        return std::unique_ptr<RustSubscriptionMonitor>(new RustSubscriptionMonitor(func, addr));
    }

} // namespace gravity
