#include "GravityDataProduct.h"
#include "GravityNode.h"
#include <iostream>
#include "shims.h"

namespace gravity
{
    RustSubscriber::RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct>&)> func) 
    {
        this->func = func;
    }

    void RustSubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
    {
        std::vector<GravityDataProduct> v;
        for (std::vector < std::shared_ptr<GravityDataProduct>>::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
        {
            GravityDataProduct to_add = **i;
            v.push_back(to_add);
        }
        // std::cout << "calling member func\n";
        this->func(v);
    }

    std::unique_ptr<RustSubscriber> newRustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&)> func)
    {
        return std::unique_ptr<RustSubscriber>(new RustSubscriber(func));
    }

    std::unique_ptr<GravityDataProduct> copyGravityDataProduct(const GravityDataProduct& gdp)
    {
        std::unique_ptr<GravityDataProduct> ret = std::unique_ptr<GravityDataProduct>(new GravityDataProduct(gdp));
        return ret;
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

    std::unique_ptr<std::string> rustGetProtoBytes(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getDataAsString()));
        // int size = gdp->getDataSize();
        // char * data = (char *) malloc(sizeof(char) * size + 1);
        // gdp->getData(data, size);
        // std::unique_ptr<std::string> ret = std::unique_ptr<std::string>(new std::string(data));
        // free(data);
        // return ret;
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
        GravitySubscriber *gs = &**&subscriber;
        return gn->subscribe(dataProductID, *gs);
    } 

}  // namespace gravity