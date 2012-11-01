/*
 * GravityServiceManager.h
 *
 *  Created on: Aug 30, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSERVICEMANAGER_H_
#define GRAVITYSERVICEMANAGER_H_

#include <zmq.h>
#include <vector>
#include "GravityNode.h"

#define SERVICE_MGR_URL "inproc://gravity_service_manager"

namespace gravity
{

using namespace std;
using namespace std::tr1;

typedef struct ServiceDetails
{
	string serviceID;
    string url;
    zmq_pollitem_t pollItem;
    GravityServiceProvider* server;
} ServiceDetails;

class GravityServiceManager
{
private:
	void* context;
	void* gravityNodeSocket;
	map<void*,shared_ptr<ServiceDetails> > serviceMapBySocket;
	map<string, shared_ptr<ServiceDetails> > serviceMapByServiceID;
	vector<zmq_pollitem_t> pollItems;
	void addService();
	void removeService();
	void ready();
public:
	/**
	 * Constructor GravityServiceManager
	 * \param context The zmq context used for connections
	 */
	GravityServiceManager(void* context);

	/**
	 * Default destructor
	 */
	virtual ~GravityServiceManager();

	/**
	 * Starts the GravityServiceManager which will manage the communication with the service client
	 */
	void start();
};

} /* namespace gravity */
#endif /* GRAVITYSERVICEMANAGER_H_ */
