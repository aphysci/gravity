#include "RustGravityNode.h"

namespace gravity {

//function for rust to be able to call constructor
    std::unique_ptr<GravityNode> newGravityNode(){

        return std::unique_ptr<GravityNode>(new GravityNode());
    }
    std::unique_ptr<GravityNode> newGravityNodeId(const std::string& componentId) 
    {
        return std::unique_ptr<GravityNode>(new GravityNode(componentId));
    }

    GravityReturnCode rustInit(const std::unique_ptr<GravityNode>& gn, const std::string& componentID)
    {
        return gn->init(componentID);
    }
    GravityReturnCode rustInit(const std::unique_ptr<GravityNode>& gn) 
    {
        return gn->init();
    }

    void rustWaitForExit(const std::unique_ptr<GravityNode>& gn) 
    {
        gn->waitForExit();
    }
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                    const std::unique_ptr<RustSubscriber>& subscriber)
    {
        return gn->subscribe(dataProductID, *subscriber);
    }
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                    const std::unique_ptr<RustSubscriber>& subscriber, const std::string& filter)
    {
        return gn->subscribe(dataProductID, *subscriber, filter);
    }
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                    const std::unique_ptr<RustSubscriber>& subscriber, const std::string& filter,
                                    const std::string& domain)
    {
        return gn->subscribe(dataProductID, *subscriber, filter, domain);
    }
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                    const std::unique_ptr<RustSubscriber>& subscriber, const std::string& filter,
                                    const std::string& domain, bool recieveLastCachedValue)
    {
        return gn->subscribe(dataProductID, *subscriber, filter, domain, recieveLastCachedValue);
    }
    GravityReturnCode rustUnsubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                      const std::unique_ptr<RustSubscriber>& subscriber)
    {
        return gn->unsubscribe(dataProductID, *subscriber);
    }

    GravityReturnCode rustPublish(const std::unique_ptr<GravityNode> &gn, const std::unique_ptr<GravityDataProduct> &gdp)
    {
        return gn->publish(*gdp);                  
    }
    GravityReturnCode rustSubscribersExist(const std::unique_ptr<GravityNode> &gn, const std::string& dataProductID, bool& hasSubscribers)
    {
        return gn->subscribersExist(dataProductID, hasSubscribers);
    }

    GravityReturnCode rustRequest(const std::unique_ptr<GravityNode>&gn, const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& dataProduct,
                                  const std::unique_ptr<RustRequestor>& requestor, const std::string& requestID,
                                  int timeout_milliseconds, const std::string& domain)
    {
        return gn->request(serviceID, *dataProduct, *requestor, requestID, timeout_milliseconds, domain);
    }

    std::shared_ptr<GravityDataProduct> rustRequestSync(const std::unique_ptr<GravityNode>& gn,
                                                    const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& request, 
                                                    int timeout_milliseconds, const std::string& domain)
    {
        return gn->request(serviceID, *request, timeout_milliseconds, domain);
    }

    GravityReturnCode rustStartHeartbeat(const std::unique_ptr<GravityNode>& gn, int64_t interval_in_microseconds)
    {
       return gn->startHeartbeat(interval_in_microseconds);
    }

    GravityReturnCode rustStopHeartbeat(const std::unique_ptr<GravityNode>& gn)
    {
       return gn->stopHeartbeat();
    }

    std::unique_ptr<std::string> rustGetStringParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, const std::string& default_value){
        return std::unique_ptr<std::string>(new std::string(gn->getStringParam(key, default_value)));
    }
    int rustGetIntParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, int default_value)
    {
        return gn->getIntParam(key, default_value);
    }
    double rustGetFloatParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, double default_value)
    {
        return gn->getFloatParam(key, default_value);
    }
    bool rustGetBoolParam(const std::unique_ptr<GravityNode>& gn, const std::string & key, bool default_value)
    {
        return gn->getBoolParam(key, default_value);
    }
    std::unique_ptr<std::string> rustGetComponentID(const std::unique_ptr<GravityNode>& gn) {
        return std::unique_ptr<std::string>(new std::string(gn->getComponentID()));
    }

    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, GravityTransportType transportType)
    {
        return gn->registerDataProduct(dataProductID, transportType);
    }

    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string & dataProductID, GravityTransportType transportType, bool cacheLastValue)
    {
        return gn->registerDataProduct(dataProductID, transportType, cacheLastValue);
    }

    GravityReturnCode rustUnregisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID)
    {
        return gn->unregisterDataProduct(dataProductID);
    }

    GravityReturnCode rustRegisterService(const std::unique_ptr<GravityNode>& gn, const std::string& serviceID,
                                          GravityTransportType transportType,
                                          const std::unique_ptr<RustServiceProvider>& server)
    {
        return gn->registerService(serviceID, transportType, *server);
    }

    GravityReturnCode rustUnregisterService(const std::unique_ptr<GravityNode>& gn, const std::string& serviceID)
    {
        return gn->unregisterService(serviceID);
    }

    GravityReturnCode rustRegisterHeartbeatListener(const std::unique_ptr<GravityNode>& gn, const std::string& componentID, int64_t interval_in_microseconds,
        const std::unique_ptr<RustHeartbeatListener>& listener, const std::string& domain)
    {
        return gn->registerHeartbeatListener(componentID, interval_in_microseconds, *listener, domain);
    }

    GravityReturnCode rustUnregisterHeartbeatListener(const std::unique_ptr<GravityNode>& gn, const std::string& componentID, const std::string& domain)
    {
        return gn->unregisterHeartbeatListener(componentID, domain);
    }

    GravityReturnCode rustRegisterRelay(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                        const std::unique_ptr<RustSubscriber>& subscriber, bool localOnly,
                                        GravityTransportType transportType)
    {
        return gn->registerRelay(dataProductID, *subscriber, localOnly, transportType);
    }

    GravityReturnCode rustRegisterRelayCache(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                             const std::unique_ptr<RustSubscriber>& subscriber, bool localOnly,
                                             GravityTransportType transportType, bool cacheLastValue)
    {
        return gn->registerRelay(dataProductID, *subscriber, localOnly, transportType, cacheLastValue);
    }

    GravityReturnCode rustUnregisterRelay(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
                                          const std::unique_ptr<RustSubscriber>& subscriber)
    {
        return gn->unregisterRelay(dataProductID, *subscriber);
    }

    std::unique_ptr<std::string> rustGetCodeString(const std::unique_ptr<GravityNode> &gn, GravityReturnCode code)
    {
        return std::unique_ptr<std::string>(new std::string(gn->getCodeString(code)));
    }

    std::unique_ptr<std::string> rustGetIP(const std::unique_ptr<GravityNode> &gn)
    {
        return std::unique_ptr<std::string>(new std::string(gn->getIP()));
    }

    std::unique_ptr<std::string> rustGetDomain(const std::unique_ptr<GravityNode>& gn)
    {
        return std::unique_ptr<std::string>(new std::string(gn->getDomain()));
    }

    std::shared_ptr<FutureResponse> rustCreateFutureResponse(const std::unique_ptr<GravityNode>& gn)
    {
        return gn->createFutureResponse();
    }

    GravityReturnCode rustSendFutureResponse(const std::unique_ptr<GravityNode>& gn,
                                             const std::shared_ptr<FutureResponse>& futureResponse)
    {
        return gn->sendFutureResponse(*futureResponse);
    }

    GravityReturnCode rustSetSubscriptionTimeoutMonitor(const std::unique_ptr<GravityNode>& gn,
                                                        const std::string& dataProductID,
                                                        const std::unique_ptr<RustSubscriptionMonitor>& monitor,
                                                        int milliSecondTimeout,
                                                        const std::string& filter, const std::string& domain)
    {
        return gn->setSubscriptionTimeoutMonitor(dataProductID, *monitor, milliSecondTimeout, filter, domain);
    }

    GravityReturnCode rustClearSubscriptionTimeoutMonitor(const std::unique_ptr<GravityNode>& gn,
                                                          const std::string& dataProductID,
                                                          const std::unique_ptr<RustSubscriptionMonitor>& monitor,
                                                          const std::string& filter, const std::string& domain)
    {
        return gn->clearSubscriptionTimeoutMonitor(dataProductID, *monitor, filter, domain);
    }
} // namespace gravity