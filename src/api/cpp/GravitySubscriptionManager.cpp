/*
 * GravitySubscriptionManager.cpp
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#include "GravitySubscriptionManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include <iostream>
#include <vector>
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
}

GravitySubscriptionManager::~GravitySubscriptionManager() {}

void GravitySubscriptionManager::start()
{
	// Messages
	zmq_msg_t filter, message;

	int ret;

	// Set up the inproc socket to subscribe to subscribe and unsubscribe messages from
	// the GravityNode
	gravityNodeSocket = zmq_socket(context, ZMQ_SUB);
	zmq_connect(gravityNodeSocket, "inproc://gravity_subscription_manager");
	zmq_setsockopt(gravityNodeSocket, ZMQ_SUBSCRIBE, NULL, 0);

	// Always have at least the gravity node to poll
	zmq_pollitem_t pollItem;
	pollItem.socket = gravityNodeSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

	ready();

	// Process forever...
	while (true)
	{
		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&pollItems[0], pollItems.size(), -1); // 0 --> return immediately, -1 --> blocks
		if (rc == -1)
		{
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

		// Check for subscription updates
		for (unsigned int i = 1; i < pollItems.size(); i++)
		{
			if (pollItems[i].revents && ZMQ_POLLIN)
			{
				// Deliver to subscriber(s)
				shared_ptr<SubscriptionDetails> subDetails = subscriptionSocketMap[pollItems[i].socket];

				vector< shared_ptr<GravityDataProduct> > dataProducts;
				while (true)
				{
					// Read data products from socket
					zmq_msg_init(&filter);
					ret = zmq_recvmsg(pollItems[i].socket, &filter, ZMQ_DONTWAIT);
					if (ret == -1)
					{
						zmq_msg_close(&filter);
						break;
					}
					int size = zmq_msg_size(&filter);
					char* s = (char*)malloc(size+1);
					memcpy(s, zmq_msg_data(&filter), size);
					s[size] = 0;
					std::string filterText(s, size);
					delete s;
					zmq_msg_close(&filter);

					zmq_msg_init(&message);
					zmq_recvmsg(pollItems[i].socket, &message, 0);
					// Create new GravityDataProduct from the incoming message
					shared_ptr<GravityDataProduct> dataProduct = shared_ptr<GravityDataProduct>(new GravityDataProduct(zmq_msg_data(&message), zmq_msg_size(&message)));
					// Clean up message
					zmq_msg_close(&message);

					shared_ptr<GravityDataProduct> lastCachedValue = lastCachedValueMap[pollItems[i].socket];

					// This may be a resend of previous value if a new subscriber was added, so make sure this is new data
					if (!lastCachedValue || lastCachedValue->getGravityTimestamp() < dataProduct->getGravityTimestamp())
					{
						dataProducts.push_back(dataProduct);

						// Save most recent value so we can provide it to new subscribers, and to perform check above.
						lastCachedValueMap[pollItems[i].socket] = dataProduct;
					}
				}

				Log::debug("received %d gdp's, about to send to %d subscribers", dataProducts.size(), subDetails->subscribers.size());

                // Loop through all subscribers and deliver the messages
                for (set<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin(); iter != subDetails->subscribers.end(); iter++)
                {
                    (*iter)->subscriptionFilled(dataProducts);
                }
			}
		}
	}

	// Clean up all our open sockets
	for (map<void*,shared_ptr<SubscriptionDetails> >::iterator iter = subscriptionSocketMap.begin(); iter != subscriptionSocketMap.end(); iter++)
	{
		zmq_close(iter->first);
	}

	subscriptionMap.clear();
	urlMap.clear();
	subscriptionSocketMap.clear();
	zmq_close(gravityNodeSocket);
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

void GravitySubscriptionManager::addSubscription()
{
	// Read the data product id for this subscription
	string dataProductID = readStringMessage(gravityNodeSocket);

	// Read the subscription url
	string url = readStringMessage(gravityNodeSocket);

	// Read the subscription filter
	string filter = readStringMessage(gravityNodeSocket);

	// Read the subscriber
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravitySubscriber* subscriber;
	memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	if (subscriptionMap.count(dataProductID) == 0)
	{
	    map<string, shared_ptr<SubscriptionDetails> > filterMap;
	    subscriptionMap[dataProductID] = filterMap;
	}

	shared_ptr<SubscriptionDetails> subDetails;
	if (subscriptionMap[dataProductID].count(filter) > 0)
	{
		// Already have a details for this
		subDetails = subscriptionMap[dataProductID][filter];
	}
	else
	{
	    subDetails.reset(new SubscriptionDetails());
        subDetails->dataProductID = dataProductID;
        subDetails->filter = filter;
	    subscriptionMap[dataProductID][filter] = subDetails;
	}

	if (subDetails->pollItemMap.count(url) == 0)
	{
		// Create the socket
		void* subSocket = zmq_socket(context, ZMQ_SUB);

		// Connect to publisher
		zmq_connect(subSocket, url.c_str());

		// Configure filter
		zmq_setsockopt(subSocket, ZMQ_SUBSCRIBE, filter.c_str(), filter.length());

		// Create poll item for this subscription
		zmq_pollitem_t pollItem;
		pollItem.socket = subSocket;
		pollItem.events = ZMQ_POLLIN;
		pollItem.fd = 0;
		pollItem.revents = 0;
		pollItems.push_back(pollItem);

		// Create subscription details
		subDetails->pollItemMap[url] = pollItem;

		// and by socket for quick lookup as data arrives
		subscriptionSocketMap[subSocket] = subDetails;
	}

	// If we've already received data on this subscription, send the most recent
	// value to the new subscriber
	vector<shared_ptr<GravityDataProduct> > dataProducts;
	for (map<string, zmq_pollitem_t>::iterator iter = subDetails->pollItemMap.begin(); iter != subDetails->pollItemMap.end(); iter++)
	{
        if (lastCachedValueMap[iter->second.socket])
            dataProducts.push_back(lastCachedValueMap[iter->second.socket]);
	}
    // Add new subscriber if it isn't already in the list
    if (subDetails->subscribers.find(subscriber) == subDetails->subscribers.end())
    {
        subDetails->subscribers.insert(subscriber);

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

	// Read the subscription filter
	string filter = readStringMessage(gravityNodeSocket);

	// Read the subscriber
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravitySubscriber* subscriber;
	memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	if (subscriptionMap.count(dataProductID) > 0 && subscriptionMap[dataProductID].count(filter) > 0)
	{
		// Get subscription details
		shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[dataProductID][filter];

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
            subscriptionMap[dataProductID].erase(filter);
            if (subscriptionMap[dataProductID].size() == 0)
                subscriptionMap.erase(dataProductID);

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

} /* namespace gravity */
