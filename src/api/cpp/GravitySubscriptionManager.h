/*
 * GravitySubscriptionManager.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSUBSCRIPTIONMANAGER_H_
#define GRAVITYSUBSCRIPTIONMANAGER_H_

#include <zmq.h>
#include <vector>
#include <map>
#ifndef __GNUC__
#include <memory>
#else
#include <tr1/memory>
#endif
#include <string>
#include "GravitySubscriber.h"

namespace gravity
{

using namespace std;
using namespace std::tr1;

typedef struct SubscriptionDetails
{
	string id;
	zmq_pollitem_t pollItem;
	vector<GravitySubscriber*> subscribers;
} SubscriptionDetails;

/**
 * The GravitySubscriptionManager is a component used internally by the GravityNode to manage
 * subscriptions in its own thread.
 */
class GravitySubscriptionManager
{
private:
	void* context;
	void* gravityNodeSocket;
	map<string,shared_ptr<SubscriptionDetails> > subscriptionMap;
    map<void*,shared_ptr<SubscriptionDetails> > subscriptionSocketMap;
	vector<zmq_pollitem_t> pollItems;

	void addSubscription();
	void removeSubscription();
	string readStringMessage();
	void ready();
	void sendStringMessage(void* socket, string str, int flags);
public:
	/**
	 * Constructor GravitySubscriptionManager
	 * \param context The zmq context in which the inproc socket will be established with the GravityNode
	 */
	GravitySubscriptionManager(void* context);

	/**
	 * Default destructor
	 */
	virtual ~GravitySubscriptionManager();

	/**
	 * Starts the GravitySubscriptionManager which will run forever, managing subscriptions and
	 * passing published data to the defined subscribers. Should be executed from GravityNode in
	 * its own thread with a shared zmq context.
	 */
	void start();
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIPTIONMANAGER_H_ */
