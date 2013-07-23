/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

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
#include "GravityMetrics.h"

namespace gravity
{

using namespace std::tr1;

typedef struct SubscriptionDetails
{
	std::string dataProductID;
	std::string filter;
	std::map<std::string, zmq_pollitem_t> pollItemMap;
	std::set<GravitySubscriber*> subscribers;
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
    void* gravityMetricsSocket;
	std::map<std::string, std::map<std::string, shared_ptr<SubscriptionDetails> > > subscriptionMap;
    std::map<void*,shared_ptr<SubscriptionDetails> > subscriptionSocketMap;
    std::map<std::string, zmq_pollitem_t> publisherUpdateMap;
    std::map<void*,shared_ptr<GravityDataProduct> > lastCachedValueMap;
	std::vector<zmq_pollitem_t> pollItems;

	void addSubscription();
	void removeSubscription();
	int readSubscription(void *socket, std::string &filterText, shared_ptr<GravityDataProduct> &dataProduct);
	void *setupSubscription(const std::string &url, const std::string &filter, zmq_pollitem_t &pollItem);
	void ready();

    bool metricsEnabled;
    GravityMetrics metricsData;
    void collectMetrics(std::vector<shared_ptr<GravityDataProduct> > dataProducts);
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
