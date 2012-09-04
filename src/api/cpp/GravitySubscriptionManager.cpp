/*
 * GravitySubscriptionManager.cpp
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#include "GravitySubscriptionManager.h"
#include <iostream>

namespace gravity
{

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
			string command = readStringMessage();

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
				// Read data products from socket
				zmq_msg_init(&filter);
				zmq_recvmsg(pollItems[i].socket, &filter, 0);
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
				GravityDataProduct dataProduct(zmq_msg_data(&message), zmq_msg_size(&message));
				// Clean up message
				zmq_msg_close(&message);

				// Deliver to subscriber(s)
				string id = dataProduct.getDataProductID() + ":" + dataProduct.getFilterText();
				shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[id];

				// Loop through all subscribers and deliver the message
				vector<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin();
				while (iter != subDetails->subscribers.end())
				{
					(*iter)->subscriptionFilled(dataProduct);
					iter++;
				}
			}
		}
	}

	// Clean up all our open sockets
	for (map<string,shared_ptr<SubscriptionDetails> >::iterator iter = subscriptionMap.begin(); iter != subscriptionMap.end(); iter++)
	{
		string id = iter->first;
		shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[id];
		zmq_close(subDetails->pollItem.socket);
	}
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

void GravitySubscriptionManager::sendStringMessage(void* socket, string str, int flags)
{
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());
	zmq_sendmsg(socket, &msg, flags);
	zmq_msg_close(&msg);
}

string GravitySubscriptionManager::readStringMessage()
{
	// Message holder
	zmq_msg_t msg;

	// Read the data product id for this subscription
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	int size = zmq_msg_size(&msg);
	char* s = (char*)malloc(size+1);
	memcpy(s, zmq_msg_data(&msg), size);
	s[size] = 0;
	std::string str(s, size);
	delete s;
	zmq_msg_close(&msg);

	return str;
}

void GravitySubscriptionManager::addSubscription()
{
	// Read the data product id for this subscription
	string dataProductID = readStringMessage();

	// Read the subscription url
	string url = readStringMessage();

	// Read the subscription filter
	string filter = readStringMessage();

	// Read the subscriber
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravitySubscriber* subscriber;
	memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	string id = dataProductID + ":" + filter;
	shared_ptr<SubscriptionDetails> subDetails;
	if (subscriptionMap.count(id))
	{
		// Already have a socket for this
		subDetails = subscriptionMap[id];
	}
	else // New subscription
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
		subDetails.reset(new SubscriptionDetails());
		subDetails->id = id;
		subDetails->pollItem = pollItem;

		// Track these subscription details by id (data product + filter)
		subscriptionMap[id] = subDetails;
	}

	// Add new subscriber
	subDetails->subscribers.push_back(subscriber);
}

void GravitySubscriptionManager::removeSubscription()
{
	// Read data product id
	string dataProductID = readStringMessage();

	// Read the subscription filter
	string filter = readStringMessage();

	// Read the subscriber
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravitySubscriber* subscriber;
	memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	// ID specific to this subscription
	string id = dataProductID + ":" + filter;

	if (subscriptionMap.count(id))
	{
		// Get subscription details
		shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[id];

		// Unsubscribe
		zmq_setsockopt(subDetails->pollItem.socket, ZMQ_UNSUBSCRIBE, filter.c_str(), filter.length());

		// Find & remove subscriber from our list of subscribers for this data product
		vector<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin();
		while (iter != subDetails->subscribers.end())
		{
			// Pointer to same subscriber?
			if (*iter == subscriber)
			{
				iter = subDetails->subscribers.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		// If no more subscribers, close the subscription socket and clear the details
		if (subDetails->subscribers.empty())
		{
			// Close the socket
			zmq_close(subDetails->pollItem.socket);

			// Remove from poll items
			vector<zmq_pollitem_t>::iterator iter = pollItems.begin();
			while (iter != pollItems.end())
			{
				if (iter->socket == subDetails->pollItem.socket)
				{
					iter = pollItems.erase(iter);
				}
				else
				{
					iter++;
				}
			}

			// Remove from details map
			subscriptionMap.erase(id);
		}
	}
}

} /* namespace gravity */
