#ifndef RUSTGRAVTITYNODE_H_
#define RUSTGRAVITYNODE_H_


#include "GravityDataProduct.h"
#include <GravityNode.h>
#include "RustGravitySubscriber.h"
#include "RustGravityRequestor.h"
#include "RustGravityHeartbeatListener.h"
#include "RustGravitySubscriptionMonitor.h"
#include "RustGravityServiceProvider.h"

namespace gravity {

    std::unique_ptr<GravityNode> newGravityNode();

    std::unique_ptr<GravityNode> newGravityNodeId(const std::string& componentId);
     
    GravityReturnCode rustInit(const std::unique_ptr<GravityNode>& gn);

    GravityReturnCode rustInit(const std::unique_ptr<GravityNode> &gn, const std::string &componentID);
    
    void rustWaitForExit(const std::unique_ptr<GravityNode>& gn);
   
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber);
    
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                    const std::string& filter);
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                    const std::string& filter, const std::string& domain);
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                    const std::string& filter, const std::string& domain, bool recieveLastCachedValue);

    GravityReturnCode rustUnsubscribe(const std::unique_ptr<GravityNode> & gn, const std::string& dataProductId, const std::unique_ptr<RustSubscriber>& subscriber);

    GravityReturnCode rustPublish(const std::unique_ptr<GravityNode> &gn, const std::unique_ptr<GravityDataProduct> &gdp);

    GravityReturnCode rustSubscribersExist(const std::unique_ptr<GravityNode> &gn, const std::string& dataProductID, bool& hasSubscribers);

    GravityReturnCode rustRequest(const std::unique_ptr<GravityNode>&gn, const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& dataProduct, 
                                const std::unique_ptr<RustRequestor>& requestor, const std::string& requestID, int timeout_milliseconds, const std::string& domain);
    
    std::shared_ptr<GravityDataProduct> rustRequestSync(const std::unique_ptr<GravityNode> &gn, const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& request, int timeout_milliseconds = -1, const std::string& domain = "");

    GravityReturnCode rustStartHeartbeat(const std::unique_ptr<GravityNode>& gn, int64_t interval_in_microseconds);
 
    GravityReturnCode rustStopHeartbeat(const std::unique_ptr<GravityNode>& gn);
    
    std::unique_ptr<std::string> rustGetStringParam(const std::unique_ptr<GravityNode>& gn, const std::string &key, const std::string& default_value = "");
    int rustGetIntParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, int default_value = -1);
    double rustGetFloatParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, double default_value = 0.0);
    bool rustGetBoolParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, bool default_value = false);
    
    std::unique_ptr<std::string> rustGetComponentID(const std::unique_ptr<GravityNode>& gn);
    
    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string & dataProductID, GravityTransportType transportType);
    
    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string & dataProductID, GravityTransportType transportType, bool cacheLastValue);

    GravityReturnCode rustUnregisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID);

    GravityReturnCode rustRegisterService(const std::unique_ptr<GravityNode>& gn, const std::string& serviceID,
                            GravityTransportType transportType, const std::unique_ptr<RustServiceProvider>& server);

    GravityReturnCode rustUnregisterService(const std::unique_ptr<GravityNode>& gn, const std::string& serviceID);

    GravityReturnCode rustRegisterRelay(const std::unique_ptr<GravityNode> & gn,
                                        const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                        bool localOnly, GravityTransportType transportType);

    
    GravityReturnCode rustRegisterHeartbeatListener(const std::unique_ptr<GravityNode>& gn, const std::string& componentID, int64_t interval_in_microseconds,
        const std::unique_ptr<RustHeartbeatListener>& listener, const std::string& domain = "");

    GravityReturnCode rustUnregisterHeartbeatListener(const std::unique_ptr<GravityNode>& gn, const std::string& componentID, const std::string& domain = "");

    GravityReturnCode rustRegisterRelayCache(const std::unique_ptr<GravityNode> & gn,
                                             const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                             bool localOnly, GravityTransportType transportType, bool cacheLastValue);

    GravityReturnCode rustUnregisterRelay(const std::unique_ptr<GravityNode> & gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber);

    std::unique_ptr<std::string> rustGetCodeString(const std::unique_ptr<GravityNode> &gn, GravityReturnCode code);
    std::unique_ptr<std::string> rustGetIP(const std::unique_ptr<GravityNode> &gn);
    std::unique_ptr<std::string> rustGetDomain(const std::unique_ptr<GravityNode> &gn);

    std::shared_ptr<FutureResponse> rustCreateFutureResponse(const std::unique_ptr<GravityNode>& gn);

    GravityReturnCode rustSendFutureResponse(const std::unique_ptr<GravityNode>& gn, const std::shared_ptr<FutureResponse>& futureResponse);
    
    GravityReturnCode rustSetSubscriptionTimeoutMonitor(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID,
        const std::unique_ptr<RustSubscriptionMonitor>& monitor, int milliSecondTimeout, const std::string& filter = "", const std::string& domain = "");

    GravityReturnCode rustClearSubscriptionTimeoutMonitor(const std::unique_ptr<GravityNode>&gn, const std::string& dataProductID,
        const std::unique_ptr<RustSubscriptionMonitor>& monitor, const std::string& filter = "", const std::string& domain = "");
    
} //namespace gravity

#endif