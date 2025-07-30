#ifndef RUSTGRAVITYSUBSCRIPTIONMONITOR_H_
#define RUSTGRAVITYSUBSCRIPTIONMONITOR_H_

#include <memory>
#include <GravitySubscriptionMonitor.h>
#include "rust/cxx.h"


namespace gravity
{
    class RustSubscriptionMonitor : public GravitySubscriptionMonitor {
        private:
            rust::Fn<void(const std::string&, int, const std::string&, const std::string&, size_t)> func;
            size_t addr;
        public:
            RustSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, size_t)> func,
                size_t addr);
            virtual void subscriptionTimeout(std::string dataProductID, int millisecondsSinceLast,
                                             std::string filter, std::string domain);
    };

    std::unique_ptr<RustSubscriptionMonitor> rustNewSubscriptionMonitor(rust::Fn<void(const std::string&, int, const std::string&, const std::string&, size_t)> func,
                size_t addr);

    
} // namespace gravity


#endif