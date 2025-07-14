#include "RustGravityDataProduct.h"

namespace gravity {

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
    
    void rustSetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& softwareVersion)
    {
        gdp->setSoftwareVersion(softwareVersion);
    }

    std::unique_ptr<std::string> rustGetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getSoftwareVersion()));
    }

    void rustSetData(const std::unique_ptr<GravityDataProduct>& gdp, const char* data, int size)
    {
        (*gdp).setData(data, size);
    }

    void rustSetDataProto(const std::unique_ptr<GravityDataProduct> &gdp, const char* data, int size, const std::string& dataType) {
        gdp->setData(data, size);
        gdp->setProtocol("protobuf2");
        gdp->setTypeName(dataType);
    }

    const char * rustGetData(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        int size = gdp->getDataSize();
        char * data = (char *) malloc(sizeof(char) * size + 1);
        gdp->getData(data, size);
        return data;
    }

    int rustGetDataSize(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->getDataSize(); 
    }

    int rustGetSize(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->getSize(); 
    }

    void rustParseFromArray(const std::unique_ptr<GravityDataProduct>& gdp, const char* arrayPtr, int size) 
    {
        gdp->parseFromArray(arrayPtr, size);
    }

    std::unique_ptr<std::vector<unsigned char>> rustSerializeToArray(const std::unique_ptr<GravityDataProduct>& gdp) {
        int size = gdp->getSize();
        void * ptr = malloc(sizeof(char) * size + 1);
        char * it = (char *) ptr;
        gdp->serializeToArray(ptr);
        std::vector<unsigned char> ret;
        for (int i = 0; i < size; i++) {
            ret.push_back(*it);
            it++;
        }
        free(ptr);
        return std::unique_ptr<std::vector<unsigned char>>(new std::vector<unsigned char>(ret));

    }

    std::unique_ptr<std::string> rustGDPGetComponentID(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getComponentId()));
    }

    

    bool rustIsFutureResponse(const std::unique_ptr<GravityDataProduct>& gdp) {
        return gdp->isFutureResponse(); 
    }

    bool rustIsCachedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->isCachedDataproduct(); 
    }

    void rustSetIsCachedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp, bool cached) 
    {
        gdp->setIsCachedDataproduct(cached);
    }

    std::unique_ptr<std::string> rustGetFutureSocketURL(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getFutureSocketUrl()));
    }

    void rustSetTimestamp(const std::unique_ptr<GravityDataProduct>& gdp, uint32_t ts) 
    {
        gdp->setTimestamp(ts);
    }

    void rustSetReceivedTimestamp(const std::unique_ptr<GravityDataProduct>& gdp, uint32_t ts) 
    {
        gdp->setReceivedTimestamp(ts);
    }

    // void rustSetComponentId(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& componentId) 
    // {
    //     std::string cid(componentId);
    //     gdp->setComponentId(cid);
    // }

    // void rustSetDomain(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& domain) 
    // {
    //     gdp->setDomain(domain);
    // }

    bool rustIsRelayedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->isRelayedDataproduct(); 
    }

    void rustSetIsRelayedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp, bool relayed)
    {
        gdp->setIsRelayedDataproduct(relayed);
    }

    void rustSetProtocol(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& protocol) 
    {
        gdp->setProtocol(protocol);
    }

    std::unique_ptr<std::string> rustGetProtocol(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getProtocol()));
    }

    void rustSetTypeName(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& dataType) 
    {
        gdp->setTypeName(dataType);
    }

    std::unique_ptr<std::string> rustGetTypeName(const std::unique_ptr<GravityDataProduct>& gdp)
    {
        return std::unique_ptr<std::string>(new std::string(gdp->getTypeName()));
    }

    uint32_t rustGetRegistrationTime(const std::unique_ptr<GravityDataProduct>& gdp) 
    { 
        return gdp->getRegistrationTime(); 
    }

    void rustSetRegistrationTime(const std::unique_ptr<GravityDataProduct>& gdp, uint32_t ts) 
    {
        gdp->setRegistrationTime(ts);
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
} // namespace gravity