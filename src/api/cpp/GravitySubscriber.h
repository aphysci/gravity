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

namespace gravity
{

class GravitySubscriber
{
public:
	virtual ~GravitySubscriber();
	virtual void subscriptionFilled(string dataProductID, vector<GravityDataProduct> dataProducts) = 0;
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIBER_H_ */
