#include "GravityDataProduct.h"
#include "GravityNode.h"

namespace gravity
{

    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const std::string& dataProductId)
    {
        return std::unique_ptr<GravityDataProduct>(new GravityDataProduct(dataProductId));
    }
    void rustSetData(const std::unique_ptr<GravityDataProduct> &gdp, const char *data, int size){
        (*gdp).setData(data, size);
    }
    void rustSetDataProto(const std::unique_ptr<GravityDataProduct> &gdp, const char* data, int size) {
        (*gdp).setDataRustInternal(data, size);
    }

    //function for rust to be able to call constructor
    std::unique_ptr<GravityNode> newGravityNode(){

        return std::unique_ptr<GravityNode>(new GravityNode());
    }

    GravityReturnCode rustInit(const std::unique_ptr<GravityNode>& gn, const std::string& componentID)
    {
        return (*gn).init(componentID);
    }

    std::unique_ptr<std::string> rustGetComponentID(const std::unique_ptr<GravityNode>& gn) {
        return std::unique_ptr<std::string>(new std::string((*gn).getComponentID()));
    }

    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, GravityTransportType transportType)
    {
        return (*gn).registerDataProduct(dataProductID, transportType);
    }

    GravityReturnCode rustPublish(const std::unique_ptr<GravityNode> &gn, const std::unique_ptr<GravityDataProduct> &gdp)
    {
        return (*gn).publish(*gdp);                  
    }



} // namespace gravity