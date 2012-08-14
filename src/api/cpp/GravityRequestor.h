/*
 * GravityRequestor.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYREQUESTOR_H_
#define GRAVITYREQUESTOR_H_

using namespace std;

namespace gravity
{

class GravityRequestor
{
public:
	virtual ~GravityRequestor();
	virtual void requestFilled(string serviceID, string requestorID, GravityDataProduct response) = 0;
};

} /* namespace gravity */


#endif /* GRAVITYREQUESTOR_H_ */
