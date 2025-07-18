#ifndef RUSTGRAVITYDATAPRODUCT_H_
#define RUSTGRAVITYDATAPRODUCT_H_


#include <string>

#include <GravityDataProduct.h>

namespace gravity
{
    std::unique_ptr<GravityDataProduct> newGravityDataProduct();

    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const std::string& dataProductId);
   
    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const char *arrayPtr, int size);
   
    uint64_t rustGetGravityTimestamp(const std::unique_ptr<GravityDataProduct>& gdp);

    uint64_t rustGetReceivedTimestamp(const std::unique_ptr<GravityDataProduct>& gdp);

    std::unique_ptr<std::string> rustGetDataProductID(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& softwareVersion);

    std::unique_ptr<std::string> rustGetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetData(const std::unique_ptr<GravityDataProduct> &gdp, const char *data, int size);

    void rustSetDataProto(const std::unique_ptr<GravityDataProduct> &gdp, const char* data, int size, const std::string& dataType);

    const char * rustGetData(const std::unique_ptr<GravityDataProduct>& gdp);

    int rustGetDataSize(const std::unique_ptr<GravityDataProduct>& gdp);

    int rustGetSize(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustParseFromArray(const std::unique_ptr<GravityDataProduct>& gdp, const char * arrayPtr, int size);

    std::unique_ptr<std::vector<unsigned char>> rustSerializeToArray(const std::unique_ptr<GravityDataProduct>& gdp);

    std::unique_ptr<std::string> rustGDPGetComponentID(const std::unique_ptr<GravityDataProduct>& gdp);

    std::unique_ptr<std::string> rustGetDomain(const std::unique_ptr<GravityDataProduct>& gdp);

    bool rustIsFutureResponse(const std::unique_ptr<GravityDataProduct>& gdp);

    bool rustIsCachedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetIsCachedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp, bool cached);

    std::unique_ptr<std::string> rustGetFutureSocketURL(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetTimestamp(const std::unique_ptr<GravityDataProduct>& gdp, uint32_t ts);

    void rustSetReceivedTimestamp(const std::unique_ptr<GravityDataProduct>& gdp, uint32_t ts);

    // void rustSetComponentId(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& componentId);

    // void rustSetDomain(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& domain);

    bool rustIsRelayedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetIsRelayedDataProduct(const std::unique_ptr<GravityDataProduct>& gdp, bool relayed);

    void rustSetProtocol(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& protocol);

    std::unique_ptr<std::string> rustGetProtocol(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetTypeName(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& dataType);

    std::unique_ptr<std::string> rustGetTypeName(const std::unique_ptr<GravityDataProduct>& gdp);

    uint32_t rustGetRegistrationTime(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetRegistrationTime(const std::unique_ptr<GravityDataProduct>& gdp, uint32_t ts);



    std::unique_ptr<GravityDataProduct> copyGravityDataProduct(const GravityDataProduct& gdp);

    std::shared_ptr<GravityDataProduct> copyGravityDataProductShared(const GravityDataProduct& gdp);
    

    void rustFree(const char * data);

} // namespace gravity


#endif