/*
 * GravityServiceProvider.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSERVICEPROVIDER_H_
#define GRAVITYSERVICEPROVIDER_H_

#include "GravityDataProduct.h"

namespace gravity
{

/**
 * Interface specification for an object that will function as the "server" side of a request-response interaction
 */
class GravityServiceProvider
{
public:
    /**
     * Default destructor
     */
    GRAVITY_API virtual ~GravityServiceProvider();

    /**
     * Called when a request is made through the Gravity infrastructure
     * \returns the response
     */
    virtual shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct) = 0;
};

} /* namespace gravity */


#endif /* GRAVITYSERVICEPROVIDER_H_ */
