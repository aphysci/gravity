/*
 * GravityRequestor.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYREQUESTOR_H_
#define GRAVITYREQUESTOR_H_

#include "GravityDataProduct.h"

using namespace std;

namespace gravity
{

/**
 * Interface specification for an object that will function as the "client" side of a request-response interaction
 */
class GravityRequestor
{
public:
	/**
	 * Default destructor
	 */
	virtual ~GravityRequestor();

	/**
	 * Called when a response to a request is received through the Gravity infrastructure
	 */
	virtual void requestFilled(string serviceID, string requestorID, GravityDataProduct response) = 0;
};

} /* namespace gravity */


#endif /* GRAVITYREQUESTOR_H_ */
