/*
 * GravityRequestManager.h
 *
 *  Created on: Aug 28, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYREQUESTMANAGER_H_
#define GRAVITYREQUESTMANAGER_H_

#include <zmq.h>
#include <vector>
#include "GravityNode.h"

namespace gravity
{

using namespace std;
using namespace std::tr1;

typedef struct RequestDetails
{
	string serviceID;
	string requestID;
	zmq_pollitem_t pollItem;
	GravityRequestor* requestor;
} RequestDetails;

class GravityRequestManager
{
private:
	void* context;
	void* gravityNodeSocket;
	map<void*,shared_ptr<RequestDetails> > requestMap;
	vector<zmq_pollitem_t> pollItems;
	string readStringMessage();
	void sendStringMessage(void* socket, string str, int flags);
	void processRequest();
	void ready();
public:
	/**
	 * Constructor GravityRequestManager
	 * \param context The zmq context in which the socket will be established with the service provider
	 */
	GravityRequestManager(void* context);

	/**
	 * Default destructor
	 */
	virtual ~GravityRequestManager();

	/**
	 * Starts the GravityRequestManager which will manage the request/response communication with
	 * the service provider
	 */
	void start();
};

} /* namespace gravity */
#endif /* GRAVITYREQUESTMANAGER_H_ */
