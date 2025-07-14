#include "GravityDataProduct.h"
#include "FutureResponse.h"
#include "GravityNode.h"
#include <iostream>
#include "shims.h"

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

    void RustHeartbeatListener::MissedHeartbeat(std::string componentID,
                int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds)
    {
        this->missed(componentID, microsecond_to_last_heartbeat, interval_in_microseconds, this->addr);
    }
    void RustHeartbeatListener::ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds)
    {
        this->received(componentID, interval_in_microseconds, this->addr);
    }
    RustHeartbeatListener::RustHeartbeatListener(rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed,
            rust::Fn<void(const std::string&, int64_t&, size_t)> received,
            size_t addr)
    {
        this->received = received;
        this->missed = missed;
        this-> addr = addr;
    }

    std::unique_ptr<RustHeartbeatListener> rustNewHeartbeatListener(
            rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed,
            rust::Fn<void(const std::string&, int64_t&, size_t)> received,
            size_t addr
        ) 
    {
        return std::unique_ptr<RustHeartbeatListener>(new RustHeartbeatListener(missed, received, addr));
    }

    


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

    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func,
        size_t addr)
    {
        return std::unique_ptr<RustServiceProvider>(new RustServiceProvider(func, addr));
    }

    


    

    RustSubscriber::RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct>&, size_t addr)> func, size_t addr) 
    {
        this->func = func;
        this->addr = addr;
    }

    void RustSubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
    {
        std::vector<GravityDataProduct> v;
        for (std::vector < std::shared_ptr<GravityDataProduct>>::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
        {
            GravityDataProduct to_add = **i;
            v.push_back(to_add);
        }
        this->func(v, this->addr);
    }

    

    std::unique_ptr<RustSubscriber> newRustSubscriber(
        rust::Fn<void(const std::vector<GravityDataProduct>&, size_t)> func, size_t addr)
    {
        return std::unique_ptr<RustSubscriber>(new RustSubscriber(func, addr));
    }

    std::unique_ptr<GravityDataProduct> copyGravityDataProduct(const GravityDataProduct& gdp)
    {
        return std::unique_ptr<GravityDataProduct>(new GravityDataProduct(gdp));
    }

    std::shared_ptr<GravityDataProduct> copyGravityDataProductShared(const GravityDataProduct& gdp)
    {
        return std::shared_ptr<GravityDataProduct>(new GravityDataProduct(gdp));
    }

    
    
    void rustFree(const char * data) {
        free((void*) data);
    }

    

    

    

    

    

    

    
   
    

    

    

    

    

    std::shared_ptr<FutureResponse> rustNewFutureResponse(const char* arrayPtr, int size)
    {
        return std::shared_ptr<FutureResponse>(new FutureResponse(arrayPtr, size));
    }

    void rustSetResponse(const std::unique_ptr<FutureResponse>& fr, const std::unique_ptr<GravityDataProduct>& response)
    {
        fr->setResponse(*response);
    }

    RustServiceProvider::RustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func,
        size_t addr)
    {
        this->func = func;
        this->addr = addr;
    }

    std::shared_ptr<GravityDataProduct> RustServiceProvider::request(const std::string serviceID,
                                                                     const GravityDataProduct& dataProduct)
    {
        return this->func(serviceID, dataProduct, this->addr);
    }

    }  // namespace gravity

void spdlog_critical(const std::string& message) {
    gravity::SpdLog::critical(message.c_str());
}
void spdlog_error(const std::string& message) {
    gravity::SpdLog::error(message.c_str());
}
void spdlog_warn(const std::string& message) {
    gravity::SpdLog::warn(message.c_str());
}
void spdlog_info(const std::string& message) {
    gravity::SpdLog::info(message.c_str());
}
void spdlog_debug(const std::string& message) {
    gravity::SpdLog::debug(message.c_str());
}
void spdlog_trace(const std::string& message) {
    gravity::SpdLog::trace(message.c_str());
}