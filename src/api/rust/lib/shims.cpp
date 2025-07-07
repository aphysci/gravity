#include "GravityDataProduct.h"
#include "GravityNode.h"
#include <iostream>
#include "shims.h"

namespace gravity
{
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
        return func(serviceID, dataProduct, this->addr);
    }

    RustRequestor::RustRequestor(
        rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> func, size_t addr)
    {
        this->func = func;
        this->addr = addr;
    }

    void RustRequestor::requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response)
    {
        func(serviceID, requestID, response, this->addr);
    }

    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(
        rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func,
        size_t addr)
    {
        return std::unique_ptr<RustServiceProvider>(new RustServiceProvider(func, addr));
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

    std::unique_ptr<RustRequestor> rustRustRequestor(
        rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> func, size_t addr)
    {
        return std::unique_ptr<RustRequestor>(new RustRequestor(func, addr));
    }

    GravityReturnCode rustRequest(const std::unique_ptr<GravityNode>&gn, const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& dataProduct,
                                  const std::unique_ptr<RustRequestor>& requestor, const std::string& requestID,
                                  int timeout_milliseconds, const std::string& domain)
    {
        return gn->request(serviceID, *dataProduct, *requestor, serviceID, timeout_milliseconds, domain);
    }

    std::shared_ptr<GravityDataProduct> rustRequestSync(const std::unique_ptr<GravityNode>& gn,
                                                    const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& request, 
                                                    int timeout_milliseconds, const std::string& domain)
    {
        return gn->request(serviceID, *request, timeout_milliseconds, domain);
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

    std::unique_ptr<RustSubscriber> newRustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr)
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

    std::unique_ptr<GravityDataProduct> newGravityDataProduct() 
    { 
        return std::unique_ptr<GravityDataProduct>(new GravityDataProduct()); 
    }

    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const std::string& dataProductId)
    {
        return std::unique_ptr<GravityDataProduct>(new GravityDataProduct(dataProductId));
    }
    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const char* arrayPtr, int size)
    {
        return std::unique_ptr<GravityDataProduct>(new GravityDataProduct((const void*) arrayPtr, size));
    }
    void rustSetData(const std::unique_ptr<GravityDataProduct>& gdp, const char* data, int size)
    {
        (*gdp).setData(data, size);
    }
    void rustSetDataProto(const std::unique_ptr<GravityDataProduct> &gdp, const char* data, int size) {
        (*gdp).setDataRustInternal(data, size);
    }

    void rustSetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp, std::string softwareVersion)
    {
        gdp->setSoftwareVersion(softwareVersion);
    }

    const char * rustGetProtoBytes(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        int size = gdp->getDataSize();
        char * data = (char *) malloc(sizeof(char) * size + 1);
        gdp->getData(data, size);
        return data;
        // std::unique_ptr<std::string> ret = std::unique_ptr<std::string>(new std::string(data));
        // free(data);
        // return ret;
    }

    int rustGetDataSize(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->getDataSize(); 
    }

    std::unique_ptr<std::string> rustGetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getSoftwareVersion()));
    }

    uint64_t rustGetGravityTimestamp(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->getGravityTimestamp();
    }

    uint64_t rustGetReceivedTimestamp(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->getReceivedTimestamp(); 
    }

    std::unique_ptr<std::string> rustGetDataProductID(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getDataProductID()));
    }

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

    std::unique_ptr<std::string> rustGetComponentID(const std::unique_ptr<GravityNode>& gn) {
        return std::unique_ptr<std::string>(new std::string(gn->getComponentID()));
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

    GravityReturnCode rustPublish(const std::unique_ptr<GravityNode> &gn, const std::unique_ptr<GravityDataProduct> &gdp)
    {
        return gn->publish(*gdp);                  
    }
    GravityReturnCode rustSubscribersExist(const std::unique_ptr<GravityNode> &gn, const std::string& dataProductID, bool& hasSubscribers)
    {
        return gn->subscribersExist(dataProductID, hasSubscribers);
    }
   
    std::unique_ptr<std::string> rustGetCodeString(const std::unique_ptr<GravityNode> &gn, GravityReturnCode code)
    {
        return std::unique_ptr<std::string>(new std::string(gn->getCodeString(code)));
    }

    std::unique_ptr<std::string> rustGetIP(const std::unique_ptr<GravityNode> &gn)
    {
        return std::unique_ptr<std::string>(new std::string(gn->getIP()));
    }

    std::unique_ptr<std::string> rustGetDomain(const std::unique_ptr<GravityNode> &gn)
    {
        return std::unique_ptr<std::string>(new std::string(gn->getDomain()));
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

    }  // namespace gravity