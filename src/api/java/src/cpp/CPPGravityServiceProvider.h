
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
    virtual shared_ptr<GravityDataProduct> request(const GravityDataProduct& dataProduct);
    virtual shared_ptr<GravityDataProduct>  request(char* array, int length, GravityDataProduct& outputResponse);
};

}
#endif /* CPPGRAVITYSERVICEPROVIDER_H_ */

