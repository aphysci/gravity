/*
 * GravityServiceProvider.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSERVICEPROVIDER_H_
#define GRAVITYSERVICEPROVIDER_H_

using namespace std;

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
	virtual ~GravityServiceProvider();

	/**
	 * Called when a request is made through the Gravity infrastructure
	 */
	virtual void request(const GravityDataProduct& dataProducts) = 0;
};

} /* namespace gravity */


#endif /* GRAVITYSERVICEPROVIDER_H_ */
