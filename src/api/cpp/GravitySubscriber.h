/*
 * GravitySubscriber.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSUBSCRIBER_H_
#define GRAVITYSUBSCRIBER_H_

#include "GravityDataProduct.h"

using namespace std;
using namespace std::tr1;

namespace gravity
{

/**
 * Interface specification for an object that will respond to subscriptions
 */
class GravitySubscriber
{
public:
	/**
	 * Default destructor
	 */
	virtual ~GravitySubscriber();

	/**
	 * Called on implementing object when a registered subscription is filled with 1 or more GravityDataProducts
	 */
	virtual void subscriptionFilled(string dataProductID, const vector<shared_ptr<GravityDataProduct> > dataProducts) = 0;
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIBER_H_ */
