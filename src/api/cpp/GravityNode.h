/*
 * GravityNode.h
 *
 *  Created on: Aug 14, 2012
 *      Author: esmf
 */

#ifndef GRAVITYNODE_H_
#define GRAVITYNODE_H_

#include <GravitySubscriber.h>
#include <GravityRequestor.h>
#include <GravityServiceProvider.h>

namespace gravity
{

using namespace std;

class GravityNode
{
private:
	uint64_t getCurrentTime();
	string getIP();
public:
	GravityNode();
	virtual ~GravityNode();

	bool registerDataProduct(string dataProductID, int networkPort, string transportType);
	bool unregisterDataProduct(string dataProductID);
	bool subscribe(string dataProductID, GravitySubscriber& subscriber, string filter = "");
	bool subscribe(string connectionURL, string dataProductID,
				   GravitySubscriber& subscriber, string filter = "");
	bool unsubscribe(string dataProductID);
	bool publish(GravityDataProduct dataProduct);
	bool request(string serviceID, GravityDataProduct dataProduct,
				 GravityRequestor& requestor, string requestID = "");
	bool request(string connectionURL, string serviceID, GravityDataProduct dataProduct,
				 GravityRequestor& requestor, string requestID = "");
	bool registerService(string serviceID, int networkPort,
						 string transportType, GravityServiceProvider& server);
};

} /* namespace gravity */
#endif /* GRAVITYNODE_H_ */
