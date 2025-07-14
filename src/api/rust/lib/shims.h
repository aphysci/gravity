#pragma once
#ifndef SHIMS_H_
#define SHIMS_H_

#include <string>

#include "GravityDataProduct.h"
#include "GravityNode.h"
#include "GravityRequestor.h"
#include "FutureResponse.h"
#include "GravityHeartbeatListener.h"
#include "rust/cxx.h"
#include "SpdLog.h"

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

    class RustHeartbeatListener : public GravityHeartbeatListener {
        private:
            rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed;
            rust::Fn<void(const std::string&, int64_t&, size_t)> received;
            size_t addr;
        public:
            RustHeartbeatListener(rust::Fn<void(
                const std::string&, int64_t, int64_t&, size_t)> missed,
                rust::Fn<void(const std::string&, int64_t&, size_t)> received,
                size_t addr
            );
            virtual void MissedHeartbeat(std::string componentID,
                int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds);
            virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds);

    };

    std::unique_ptr<RustHeartbeatListener> rustNewHeartbeatListener(
            rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed,
            rust::Fn<void(const std::string&, int64_t&, size_t)> received,
            size_t addr
        );

    
    class RustServiceProvider : public GravityServiceProvider {
        private:    
            rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func;
            size_t addr;
        public:
            RustServiceProvider(rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func, size_t addr);
            virtual std::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    };

    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func, size_t addr);

    
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

   
    class RustSubscriber : public GravitySubscriber {
        private:
            rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func;
            size_t addr;
        public:
            RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr);
            virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
    };

    
        std::unique_ptr<RustSubscriber> newRustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr);
    
    std::unique_ptr<GravityDataProduct> copyGravityDataProduct(const GravityDataProduct& gdp);

    std::shared_ptr<GravityDataProduct> copyGravityDataProductShared(const GravityDataProduct& gdp);
    

    /** Rust-able default constructor
     * 
     */
     
    /**
     * setData callable by rust
     * Set the application-specific data for this data product
     * \param data pointer to arbitrary data
     * \param size length of data
     */

    
    std::unique_ptr<std::string> rustGDPGetComponentID(const std::unique_ptr<GravityDataProduct>& gdp);

    std::unique_ptr<std::string> rustGetDomain(const std::unique_ptr<GravityDataProduct>& gdp);

    bool rustIsFutureResponse(const std::unique_ptr<GravityDataProduct>& gdp);

    bool rustIsCachedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetIsCachedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp, bool cached);

    std::unique_ptr<std::string> rustGetFutureSocketURL(const std::unique_ptr<GravityDataProduct>& gdp);

    bool rustIsRelayedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetIsRelayedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp, bool relayed);

    void rustSetProtocol(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& protocol);

    std::unique_ptr<std::string> rustGetProtocol(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetTypeName(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& dataType);

    std::unique_ptr<std::string> rustGetTypeName(const std::unique_ptr<GravityDataProduct>& gdp);

    uint32_t rustGetRegistrationTime(const std::unique_ptr<GravityDataProduct>& gdp);



    void rustFree(const char * data);
    /**
     * Constructor, callable by Rust
     * \return pointer to a GravityNode
     */
    
    
    
 /**
     * Register a data product with the Gravity, and optionally, the Directory Service, making it available to the
     * rest of the Gravity-enabled system.
     * Callable by Rust.
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param transportType transport type (e.g. 'tcp', 'ipc')
     * \return success flag
     */
    

   /* Future response things*/
    std::shared_ptr<FutureResponse> rustNewFutureResponse(const char* arrayPtr, int size);

    void rustSetResponse(const std::unique_ptr<FutureResponse>& fr, const std::unique_ptr<GravityDataProduct>& response);

    

} // namespace gravity

void spdlog_critical(const std::string& message);
void spdlog_error(const std::string& message);;
void spdlog_warn(const std::string& message);
void spdlog_info(const std::string& message);
void spdlog_debug(const std::string& message);
void spdlog_trace(const std::string& message);

#endif