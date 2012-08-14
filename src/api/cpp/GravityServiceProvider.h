/*
 * GravityServiceProvider.h
 *
 *  Created on: Aug 14, 2012
 *      Author: esmf
 */

#ifndef GRAVITYSERVICEPROVIDER_H_
#define GRAVITYSERVICEPROVIDER_H_

using namespace std;

namespace gravity
{

class GravityServiceProvider
{
public:
	virtual ~GravityServiceProvider();
	virtual void request(GravityDataProduct dataProducts) = 0;
};

} /* namespace gravity */


#endif /* GRAVITYSERVICEPROVIDER_H_ */
