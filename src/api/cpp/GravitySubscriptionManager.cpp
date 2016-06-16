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
 * GravitySubscriptionManager.cpp
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#include "GravitySubscriptionManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include "GravityMetricsUtil.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"

#include <memory>
#include <algorithm>

namespace gravity
{

bool sortCacheValues (const shared_ptr<GravityDataProduct> &i, const shared_ptr<GravityDataProduct> &j)
{
    return i->getGravityTimestamp() < j->getGravityTimestamp();
}


using namespace std;

GravitySubscriptionManager::GravitySubscriptionManager(void* context)
{
	// This is the zmq context that is shared with the GravityNode. Must use
	// a shared context to establish an inproc socket.
	this->context = context;

    // Default to no metrics
    metricsEnabled = false;
}

GravitySubscriptionManager::~GravitySubscriptionManager() {}

void GravitySubscriptionManager::start()
{
    vector<void*> deleteList;

    // Set up the inproc socket to subscribe to subscribe and unsubscribe messages from
	// the GravityNode
	gravityNodeSocket = zmq_socket(context, ZMQ_SUB);
	zmq_connect(gravityNodeSocket, "inproc://gravity_subscription_manager");
	zmq_setsockopt(gravityNodeSocket, ZMQ_SUBSCRIBE, NULL, 0);

    // Setup socket to reponsd to metrics requests
    gravityMetricsSocket = zmq_socket(context, ZMQ_REP);
    zmq_bind(gravityMetricsSocket, GRAVITY_SUB_METRICS_REQ);

	// Poll the gravity node
	zmq_pollitem_t pollItem;
	pollItem.socket = gravityNodeSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

    // Poll the metrics request socket
    zmq_pollitem_t metricsRequestPollItem;
    metricsRequestPollItem.socket = gravityMetricsSocket;
    metricsRequestPollItem.events = ZMQ_POLLIN;
    metricsRequestPollItem.fd = 0;
    metricsRequestPollItem.revents = 0;
    pollItems.push_back(metricsRequestPollItem);

	ready();

	// Process forever...
	while (true)
	{
		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&pollItems[0], pollItems.size(), -1); // 0 --> return immediately, -1 --> blocks
		if (rc == -1)
		{
		    Log::debug("Interrupted, exiting (rc = %d)", rc);
			// Interrupted
			break;
		}

		// Process new subscription requests from the gravity node
		if (pollItems[0].revents & ZMQ_POLLIN)
		{
			// Get new GravityNode request
			string command = readStringMessage(gravityNodeSocket);

			// message from gravity node should be either a subscribe or unsubscribe request
			if (command == "subscribe")
			{
				addSubscription();
			}
			else if (command == "unsubscribe")
			{
				removeSubscription();
			}
			else if (command == "kill")
			{
				break;
			}
			else
			{
				// LOG WARNING HERE - Unknown command request
			}
		}

        if (pollItems[1].revents & ZMQ_POLLIN)
        {
            // Received a command from the metrics control
            void* socket = pollItems[1].socket;
            string command = readStringMessage(socket);

            if (command == "MetricsEnable")
            {
                // Clear any metrics data
                metricsData.reset();

                // Enable metrics
                metricsEnabled = true;

                // Acknowledge message
                sendStringMessage(socket, "ACK", ZMQ_DONTWAIT);
            }
            else if (command == "MetricsDisable")
            {
                // Disable metrics
                metricsEnabled = false;

                // Acknowledge message
                sendStringMessage(socket, "ACK", ZMQ_DONTWAIT);
            }
            else if (command == "GetMetrics")
            {
                // The GravityMetricsManager has request our metrics data

                // Mark the collection as completed
                metricsData.done();

                // Respond with metrics
                metricsData.sendAsMessage(socket);

                // Clear metrics data
                metricsData.reset();
            }
        }

		// Check for subscription updates
		for (unsigned int index = 2; index < pollItems.size(); index++)
		{
	        Log::trace("about to check for new poll items, index = %d, size = %d", index, pollItems.size());
			if (pollItems[index].revents && ZMQ_POLLIN)
			{
			    // if it's a regular subscription poll item
			    if (subscriptionSocketMap.count(pollItems[index].socket) > 0)
			    {
                    // Deliver to subscriber(s)
                    shared_ptr<SubscriptionDetails> subDetails = subscriptionSocketMap[pollItems[index].socket];

                    vector< shared_ptr<GravityDataProduct> > dataProducts;
                    while (true)
                    {
                        string filterText;
                        shared_ptr<GravityDataProduct> dataProduct;
                        if (readSubscription(pollItems[index].socket, filterText, dataProduct) < 0)
                            break;

                        shared_ptr<GravityDataProduct> lastCachedValue = lastCachedValueMap[pollItems[index].socket];

                        // This may be a resend of previous value if a new subscriber was added, so make sure this is new data
                        if (!lastCachedValue ||
                            lastCachedValue->getGravityTimestamp() < dataProduct->getGravityTimestamp() ||
                            // or if timestamps are the same, but GDP's are different
                               (lastCachedValue->getGravityTimestamp() == dataProduct->getGravityTimestamp()
                                && !(*lastCachedValue == *dataProduct)))
                        {
                            dataProducts.push_back(dataProduct);

                            // Save most recent value so we can provide it to new subscribers, and to perform check above.
                            lastCachedValueMap[pollItems[index].socket] = dataProduct;
                        }
                    }

                    Log::trace("received %d gdp's, about to send to %d subscribers", dataProducts.size(), subDetails->subscribers.size());

                    // Loop through all subscribers and deliver the messages
					if(dataProducts.size() != 0)
					{
						for (set<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin(); iter != subDetails->subscribers.end(); iter++)
						{
							(*iter)->subscriptionFilled(dataProducts);
						}

                        if (metricsEnabled)
                        {
                            collectMetrics(dataProducts);
                        }
					}
                }
			    else // it's a publisher update list from the SD
			    {
                    shared_ptr<GravityDataProduct> dataProduct;
                    string dataProductID;
                    readSubscription(pollItems[index].socket, dataProductID, dataProduct);
                    if (dataProductID.empty())
                    {
                        Log::warning("Received an apparently empty update list from ServiceDirectory");
                        continue;
                    }
                    ComponentDataLookupResponsePB update;
                    dataProduct->populateMessage(update);
					string domain = update.domain_id();
                    Log::trace("Found update to publishers list for data product %s (domain:%s)", dataProductID.c_str(), domain.c_str());

					// Create the domain/data key for tracking subscriptions
					DomainDataKey key(domain, dataProductID);

					Log::trace("subscriptionMap.count(key) = %d", subscriptionMap.count(key));
                    if (subscriptionMap.count(key) != 0)
                    {
                        for (map<string, shared_ptr<SubscriptionDetails> >::iterator iter = subscriptionMap[key].begin();
                             iter != subscriptionMap[key].end();
                             iter++)
                        {
                            for (int i = 0; i < update.url_size(); i++)
                            {
                                Log::trace("url: %s", update.url(i).c_str());

                                // if we don't already have this url, add it
                                if (iter->second->pollItemMap.count(update.url(i)) == 0)
                                {
                                    zmq_pollitem_t pollItem;
                                    void *subSocket = setupSubscription(update.url(i), iter->first, pollItem);

                                    // and by socket for quick lookup as data arrives
                                    subscriptionSocketMap[subSocket] = iter->second;

                                    // Add url & poll item to subscription details
                                    iter->second->pollItemMap[update.url(i)] = pollItem;
                                }
                            }

                            // loop through the existing urls/sockets to see if any have disappeared.
                            map<string, zmq_pollitem_t>::iterator socketIter = iter->second->pollItemMap.begin();
                            while (socketIter != iter->second->pollItemMap.end())
                            {
                                bool found = false;
                                // there doesn't seem to be a good way to check containment in a protobuf set...
                                for (int i = 0; i < update.url_size(); i++)
                                {
                                    if (socketIter->first == update.url(i))
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found) {
                                    Log::debug("url %s is gone, removing from list", socketIter->first.c_str());

                                    subscriptionSocketMap.erase(socketIter->second.socket);
                                    deleteList.push_back(socketIter->second.socket);

                                    // Unsubscribe
                                    zmq_setsockopt(socketIter->second.socket, ZMQ_UNSUBSCRIBE, iter->second->filter.c_str(), iter->second->filter.length());

                                    // Close the socket
                                    zmq_close(socketIter->second.socket);

                                    iter->second->pollItemMap.erase(socketIter++);

                                } else
                                    ++socketIter;
                            }

                            Log::trace("There are now %d (%d) sockets we're listening on", subscriptionSocketMap.size(), iter->second->pollItemMap.size());
                        }
                    }
			    }
			}
		}

		if(deleteList.size() > 0)
		{
		    for (vector<void*>::iterator iter = deleteList.begin(); iter != deleteList.end(); iter++)
		    {
		        for (vector<zmq_pollitem_t>::iterator pollIter = pollItems.begin(); pollIter != pollItems.end(); ++pollIter)
		            if (*iter == pollIter->socket)
		            {
		                pollItems.erase(pollIter);
                        Log::debug("delete socket from pollitems, size is now %d", pollItems.size());
		                break;
		            }
		    }
		    deleteList.clear();
		}
	}

	// Clean up all our open sockets
	for (map<void*,shared_ptr<SubscriptionDetails> >::iterator iter = subscriptionSocketMap.begin(); iter != subscriptionSocketMap.end(); iter++)
	{
		zmq_close(iter->first);
	}

	for (map<DomainDataKey, zmq_pollitem_t>::iterator iter = publisherUpdateMap.begin(); iter != publisherUpdateMap.end(); iter++)
	{
	    zmq_close(iter->second.socket);
	}

	subscriptionMap.clear();
	subscriptionSocketMap.clear();
	publisherUpdateMap.clear();
	zmq_close(gravityNodeSocket);
    zmq_close(gravityMetricsSocket);
}

void GravitySubscriptionManager::ready()
{
	// Create the request socket
	void* initSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(initSocket, "inproc://gravity_init");

	// Send request to service provider
	sendStringMessage(initSocket, "GravitySubscriptionManager", ZMQ_DONTWAIT);

	zmq_close(initSocket);
}

int GravitySubscriptionManager::readSubscription(void *socket, string &filterText, shared_ptr<GravityDataProduct> &dataProduct)
{
    // Messages
    zmq_msg_t filter, message;

    int ret = 0;

    // Read data products from socket
    zmq_msg_init(&filter);
    ret = zmq_recvmsg(socket, &filter, ZMQ_DONTWAIT);
    if (ret == -1)
    {
        zmq_msg_close(&filter);
        return ret;
    }
    int size = zmq_msg_size(&filter);
    char* s = (char*)malloc(size+1);
    memcpy(s, zmq_msg_data(&filter), size);
    s[size] = 0;
    filterText = string(s, size);
    free(s);
    zmq_msg_close(&filter);

    zmq_msg_init(&message);
    zmq_recvmsg(socket, &message, 0);
    // Create new GravityDataProduct from the incoming message
    dataProduct = shared_ptr<GravityDataProduct>(new GravityDataProduct(zmq_msg_data(&message), zmq_msg_size(&message)));
    // Clean up message
    zmq_msg_close(&message);

    return ret;
}

void *GravitySubscriptionManager::setupSubscription(const string &url, const string &filter, zmq_pollitem_t &pollItem)
{
    // Create the socket
    void* subSocket = zmq_socket(context, ZMQ_SUB);

    // Connect to publisher
    zmq_connect(subSocket, url.c_str());

    // Configure filter
    zmq_setsockopt(subSocket, ZMQ_SUBSCRIBE, filter.c_str(), filter.length());

    // set up the poll item for this subscription
    pollItem.socket = subSocket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;
    pollItems.push_back(pollItem);

    return subSocket;
}

void GravitySubscriptionManager::addSubscription()
{
	// Read the data product id for this subscription
	string dataProductID = readStringMessage(gravityNodeSocket);
	Log::trace("dataProductID = '%s'", dataProductID.c_str());

	// Read the subscription url
	string url = readStringMessage(gravityNodeSocket);
	Log::trace("url = '%s'", url.c_str());

	// Read the subscription filter
	string filter = readStringMessage(gravityNodeSocket);
	Log::trace("filter = '%s'", filter.c_str());

	// Read the domain
	string domain = readStringMessage(gravityNodeSocket);
	Log::trace("domain = '%s'", domain.c_str());

    // Read the url for to subscribe to for publisher updates
    string publisherUpdateUrl = readStringMessage(gravityNodeSocket);
	Log::trace("publisherUpdateUrl = '%s'", publisherUpdateUrl.c_str());

	// Read the subscriber
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravitySubscriber* subscriber;
	memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	// Create the domain/data key for tracking subscriptions
	DomainDataKey key(domain, dataProductID);

	if (subscriptionMap.count(key) == 0)
	{
	    map<string, shared_ptr<SubscriptionDetails> > filterMap;
	    subscriptionMap[key] = filterMap;
	    if (publisherUpdateUrl.size() > 0)
	    {
            zmq_pollitem_t pollItem;
            setupSubscription(publisherUpdateUrl, dataProductID, pollItem);
            publisherUpdateMap[key] = pollItem;
	    }
	}

	shared_ptr<SubscriptionDetails> subDetails;
	if (subscriptionMap[key].count(filter) > 0)
	{
		// Already have a details for this
		subDetails = subscriptionMap[key][filter];
	}
	else
	{
	    subDetails.reset(new SubscriptionDetails());
        subDetails->dataProductID = dataProductID;
		subDetails->domain = domain;
        subDetails->filter = filter;
	    subscriptionMap[key][filter] = subDetails;
	}

	// if we have a url and we haven't seen it before, subscribe to it
	if (url.size() > 0 && subDetails->pollItemMap.count(url) == 0)
	{
	    zmq_pollitem_t pollItem;
	    void *subSocket = setupSubscription(url, filter, pollItem);

		// Create subscription details
		subDetails->pollItemMap[url] = pollItem;

		// and by socket for quick lookup as data arrives
		subscriptionSocketMap[subSocket] = subDetails;
	}

    // Add new subscriber if it isn't already in the list
    if (subDetails->subscribers.find(subscriber) == subDetails->subscribers.end())
    {
        subDetails->subscribers.insert(subscriber);

        // If we've already received data on this subscription, send the most recent
        // value to the new subscriber
        vector<shared_ptr<GravityDataProduct> > dataProducts;
        for (map<string, zmq_pollitem_t>::iterator iter = subDetails->pollItemMap.begin(); iter != subDetails->pollItemMap.end(); iter++)
        {
            if (lastCachedValueMap[iter->second.socket])
                dataProducts.push_back(lastCachedValueMap[iter->second.socket]);
        }
        if (dataProducts.size() > 0)
        {
            Log::debug("sending data (%s) to late subscriber", dataProductID.c_str());
            sort(dataProducts.begin(), dataProducts.end(), sortCacheValues);
            subscriber->subscriptionFilled(dataProducts);
        }
    }
}

void GravitySubscriptionManager::removeSubscription()
{
	// Read data product id
	string dataProductID = readStringMessage(gravityNodeSocket);

    // Remove this data product from our metrics
    metricsData.remove(dataProductID);

	// Read the subscription filter
	string filter = readStringMessage(gravityNodeSocket);

	// Read the domain
	string domain = readStringMessage(gravityNodeSocket);

	// Read the subscriber
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravitySubscriber* subscriber;
	memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	// Create the domain/data key for tracking subscriptions
	DomainDataKey key(domain, dataProductID);

	if (subscriptionMap.count(key) > 0 && subscriptionMap[key].count(filter) > 0)
	{
		// Get subscription details
		shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[key][filter];

		// Find & remove subscriber from our list of subscribers for this data product
		set<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin();
		while (iter != subDetails->subscribers.end())
		{
			// Pointer to same subscriber?
			if (*iter == subscriber)
			{
				subDetails->subscribers.erase(iter);
				break;
			}
			else
			{
				iter++;
			}
		}

		// If no more subscribers, close the subscription sockets and clear the details
		if (subDetails->subscribers.empty())
		{
            // Remove from details main map
            subscriptionMap[key].erase(filter);
            if (subscriptionMap[key].size() == 0)
                subscriptionMap.erase(key);

            for (map<string, zmq_pollitem_t>::iterator iter = subDetails->pollItemMap.begin(); iter != subDetails->pollItemMap.end(); iter++)
            {
                subscriptionSocketMap.erase(iter->second.socket);

                // Unsubscribe
                zmq_setsockopt(iter->second.socket, ZMQ_UNSUBSCRIBE, filter.c_str(), filter.length());

                // Close the socket
                zmq_close(iter->second.socket);

                // Remove from poll items
                vector<zmq_pollitem_t>::iterator pollIter = pollItems.begin();
                while (pollIter != pollItems.end())
                {
                    if (pollIter->socket == iter->second.socket)
                    {
                        pollIter = pollItems.erase(pollIter);
                    }
                    else
                    {
                        pollIter++;
                    }
                }
            }
		}
	}
}

void GravitySubscriptionManager::collectMetrics(vector<shared_ptr<GravityDataProduct> > dataProducts)
{
    // Iterate over all the data products
    vector<shared_ptr<GravityDataProduct> >::iterator gdpIter;
    for (gdpIter = dataProducts.begin(); gdpIter != dataProducts.end(); gdpIter++)
    {
        shared_ptr<GravityDataProduct> gdp = *gdpIter;
        metricsData.incrementMessageCount(gdp->getDataProductID(), 1);
        metricsData.incrementByteCount(gdp->getDataProductID(), gdp->getSize());
    }
}

} /* namespace gravity */
