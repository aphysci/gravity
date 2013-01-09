
#ifndef CPPGRAVITYSERVICEPROVIDER_H_
#define CPPGRAVITYSERVICEPROVIDER_H_

#include "GravityServiceProvider.h"

namespace gravity
{

/**
 * Native implementation of a GravityServiceProvider
 */
class CPPGravityServiceProvider : public GravityServiceProvider
{
public:

    virtual ~CPPGravityServiceProvider();
    virtual shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    virtual shared_ptr<GravityDataProduct>  request(const std::string serviceID, char* array, int length);
};

}
#endif /* CPPGRAVITYSERVICEPROVIDER_H_ */

