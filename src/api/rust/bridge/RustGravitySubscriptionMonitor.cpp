#include "RustGravitySubscriptionMonitor.h"
#include "gravity/src/api/rust/src/ffi.rs.h"

namespace gravity
{
    void RustSubscriptionMonitor::subscriptionTimeout(std::string dataProductID, int millisecondsSinceLast,
                                                 std::string filter, std::string domain)
    {
        this->func(dataProductID, millisecondsSinceLast, filter, domain, this->monitorPtr);
    }

    RustSubscriptionMonitor::RustSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, MonitorWrap *)> func, MonitorWrap * monitorPtr) : monitorPtr(monitorPtr)
    {
        this->func = func;
    }
    
    std::unique_ptr<RustSubscriptionMonitor> rustNewSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, MonitorWrap *)> func, MonitorWrap * monitorPtr)
    {
        return std::unique_ptr<RustSubscriptionMonitor>(new RustSubscriptionMonitor(func, monitorPtr));
    }

} // namespace gravity
