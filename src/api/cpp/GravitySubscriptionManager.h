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
#include <set>
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
	string dataProductID;
	string filter;
	map<string, zmq_pollitem_t> pollItemMap;
	set<GravitySubscriber*> subscribers;
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
	map<string, map<string, shared_ptr<SubscriptionDetails> > > subscriptionMap;
    map<void*,shared_ptr<SubscriptionDetails> > subscriptionSocketMap;
    map<string, zmq_pollitem_t> publisherUpdateMap;
    map<void*,shared_ptr<GravityDataProduct> > lastCachedValueMap;
	vector<zmq_pollitem_t> pollItems;

	void addSubscription();
	void removeSubscription();
	int readSubscription(void *socket, string &filterText, shared_ptr<GravityDataProduct> &dataProduct);
	void *setupSubscription(const string &url, const string &filter, zmq_pollitem_t &pollItem);
	void ready();
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
