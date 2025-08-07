#ifndef RUSTGRAVITYSUBSCRIPTIONMONITOR_H_
#define RUSTGRAVITYSUBSCRIPTIONMONITOR_H_

#include <memory>
#include <GravitySubscriptionMonitor.h>
#include "rust/cxx.h"

struct MonitorWrap;

namespace gravity
{
    class RustSubscriptionMonitor : public GravitySubscriptionMonitor {
        private:
            rust::Fn<void(const std::string&, int, const std::string&, const std::string&, MonitorWrap *)> func;
            MonitorWrap * monitorPtr;
        public:
            RustSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, MonitorWrap *)> func,
                MonitorWrap * monitorPtr);
            virtual void subscriptionTimeout(std::string dataProductID, int millisecondsSinceLast,
                                             std::string filter, std::string domain);
    };

    std::unique_ptr<RustSubscriptionMonitor> rustNewSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, MonitorWrap *)> func,
                MonitorWrap * monitorPtr);

    
} // namespace gravity


#endif