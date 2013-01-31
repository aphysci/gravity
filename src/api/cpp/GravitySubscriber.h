/*
 * GravitySubscriber.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSUBSCRIBER_H_
#define GRAVITYSUBSCRIBER_H_

#include "GravityDataProduct.h"

namespace gravity
{

using namespace std::tr1;

/**
 * Interface specification for an object that will respond to subscriptions
 */
class GravitySubscriber
{
public:
    /**
     * Default destructor
     */
    GRAVITY_API virtual ~GravitySubscriber();

    /**
     * Called on implementing object when a registered subscription is filled with 1 or more GravityDataProducts
     */
    virtual void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts) = 0;
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIBER_H_ */
