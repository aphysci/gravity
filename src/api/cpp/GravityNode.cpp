/*
 * GravityNode.cpp
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#include "GravityNode.h"

#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace gravity
{

GravityNode::GravityNode() {}

GravityNode::~GravityNode() {}

bool GravityNode::registerDataProduct(string dataProductID, unsigned short networkPort, string transportType)
{
	return false;
}

bool GravityNode::unregisterDataProduct(string dataProductID)
{
	return false;
}

bool GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter)
{
	return false;
}

bool GravityNode::subscribe(string connectionURL, string dataProductID,
						    const GravitySubscriber& subscriber, string filter)
{
	return false;
}

bool GravityNode::unsubscribe(string dataProductID, const GravitySubscriber& subscriber)
{
	return false;
}

bool GravityNode::publish(const GravityDataProduct& dataProduct)
{
	return false;
}

bool GravityNode::request(string serviceID, const GravityDataProduct& dataProduct,
						  const GravityRequestor& requestor, string requestID)
{
	return false;
}

bool GravityNode::request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
						  const GravityRequestor& requestor, string requestID)
{
	return false;
}

bool GravityNode::registerService(string serviceID, unsigned short networkPort,
								  string transportType, const GravityServiceProvider& server)
{
	return false;
}

bool GravityNode::unregisterService(string serviceID, const GravityServiceProvider& server)
{
	return false;
}

uint64_t GravityNode::getCurrentTime()
{
	timespec ts;
	clock_gettime(0, &ts);
	return (uint64_t)ts.tv_sec * 1000000LL + (uint64_t)ts.tv_nsec / 1000LL;
}

string GravityNode::getIP()
{
	string ip;

	int buflen = 16;
	char* buffer = (char*)malloc(buflen);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock != -1);

	const char* otherIP = "192.168.2.44";
	uint16_t otherPort = 5555;

	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(otherIP);
	serv.sin_port = htons(otherPort);

	int err = connect(sock, (const sockaddr*)&serv, sizeof(serv));
	assert(err != -1);

	sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (sockaddr*)&name, &namelen);
	assert(err != -1);

	const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
	assert(p);

	close(sock);

	ip.assign(buffer);
	return ip;
}

} /* namespace gravity */
