/*
 * GravityPublishManager.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Mark Barger
 */

#include "GravityPublishManager.h"
#include "GravityLogger.h"
#include "zmq.h"
#include <iostream>

namespace gravity
{

using namespace std;

GravityPublishManager::GravityPublishManager(void* context)
{
	// This is the zmq context that is shared with the GravityNode. Must use
	// a shared context to establish an inproc socket.
	this->context = context;
}

GravityPublishManager::~GravityPublishManager() {}

void GravityPublishManager::start()
{
	// Messages
	zmq_msg_t filter, message;

	// Set up the inproc socket to subscribe and unsubscribe messages from
	// the GravityNode
	gravityNodeSocket = zmq_socket(context, ZMQ_SUB);
	zmq_connect(gravityNodeSocket, "inproc://gravity_publish_manager");
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
		Log::warning("pub mgr in main loop");
		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&pollItems[0], pollItems.size(), -1); // 0 --> return immediately, -1 --> blocks
		Log::warning("pub mgr in main loop 2");
		if (rc == -1)
		{
			// Interrupted
			break;
		}

		// Process new requests from the gravity node
		if (pollItems[0].revents & ZMQ_POLLIN)
		{
			Log::warning("pub mgr received msg");
			// Get new GravityNode request
			string command = readStringMessage();
			Log::warning("pub mgr received msg: %s", command.c_str());

			// message from gravity node should be either a register, unregister or publish request
			if (command == "register")
			{
				registerDataProduct();
			}
			else if (command == "unregister")
			{
				unregisterDataProduct();
			}
			else if (command == "publish")
			{
				publish();
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

		// Check for publish updates
		for (unsigned int i = 1; i < pollItems.size(); i++)
		{
			if (pollItems[i].revents && ZMQ_POLLIN)
			{
//				// Read data products from socket
//				zmq_msg_init(&filter);
//				zmq_recvmsg(pollItems[i].socket, &filter, 0);
//				int size = zmq_msg_size(&filter);
//				char* s = (char*)malloc(size+1);
//				memcpy(s, zmq_msg_data(&filter), size);
//				s[size] = 0;
//				std::string filterText(s, size);
//				delete s;
//				zmq_msg_close(&filter);
//
//				zmq_msg_init(&message);
//				zmq_recvmsg(pollItems[i].socket, &message, 0);
//				// Create new GravityDataProduct from the incoming message
//				GravityDataProduct dataProduct(zmq_msg_data(&message), zmq_msg_size(&message));
//				// Clean up message
//				zmq_msg_close(&message);
//
//				// Deliver to subscriber(s)
//				shared_ptr<PublishDetails> pubDetails = publishMapBySocket[pollItems[i].socket];
			}
		}
	}

	// Clean up any pub sockets
	for (map<void*,shared_ptr<PublishDetails> >::iterator iter = publishMapBySocket.begin(); iter != publishMapBySocket.end(); iter++)
	{
		shared_ptr<PublishDetails> pubDetails = publishMapBySocket[iter->second->socket];
		zmq_close(pubDetails->pollItem.socket);
	}

	publishMapBySocket.clear();
	publishMapByID.clear();

	zmq_close(gravityNodeSocket);
}

void GravityPublishManager::ready()
{
	// Create the request socket
	void* initSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(initSocket, "inproc://gravity_init");

	// Send request to service provider
	sendStringMessage(initSocket, "GravityPublishManager", ZMQ_DONTWAIT);

	Log::warning("pub mgr started");

	zmq_close(initSocket);
}

void GravityPublishManager::sendStringMessage(void* socket, string str, int flags)
{
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());
	zmq_sendmsg(socket, &msg, flags);
	zmq_msg_close(&msg);
}

string GravityPublishManager::readStringMessage()
{
	// Message holder
	zmq_msg_t msg;

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

void GravityPublishManager::registerDataProduct()
{

	// Read the data product id for this request
	string dataProductID = readStringMessage();

	// Read the publish url
	string connectionURL = readStringMessage();

	Log::message("received register request for %s at %s", dataProductID.c_str(), connectionURL.c_str());

    // Create the publish socket
    void* pubSocket = zmq_socket(context, ZMQ_XPUB);
    if (!pubSocket)
    {
        return;
    }

    // Bind socket to url
    int rc = zmq_bind(pubSocket, connectionURL.c_str());
    if (rc != 0)
    {
        zmq_close(pubSocket);
        Log::critical("Could not bind address %s", connectionURL.c_str());
        return;
    }

	// Create poll item for response to this request
	zmq_pollitem_t pollItem;
	pollItem.socket = pubSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

    // Track dataProductID->socket mapping
    shared_ptr<PublishDetails> publishDetails = shared_ptr<PublishDetails>(new PublishDetails);
    publishDetails->url = connectionURL;
    publishDetails->socket = pubSocket;
	publishDetails->pollItem = pollItem;

    publishMapByID[dataProductID] = publishDetails;
    publishMapBySocket[pubSocket] = publishDetails;
}

void GravityPublishManager::unregisterDataProduct()
{
	// Read the data product id for this request
	string dataProductID = readStringMessage();

	// If data product ID exists, clean up and remove socket. Otherwise, likely a duplicate unregister request
	if (publishMapByID.count(dataProductID))
	{
		shared_ptr<PublishDetails> publishDetails = publishMapByID[dataProductID];
		void* socket = publishDetails->pollItem.socket;
		publishMapBySocket.erase(socket);
		publishMapByID.erase(dataProductID);
		zmq_unbind(socket, publishDetails->url.c_str());
		zmq_close(socket);

		// Remove from poll items
		vector<zmq_pollitem_t>::iterator iter = pollItems.begin();
		while (iter != pollItems.end())
		{
			if (iter->socket == socket)
			{
				iter = pollItems.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
}

void GravityPublishManager::publish()
{
	// Read the filter text
	string filterText = readStringMessage();

	// Read the data product
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityDataProduct dataProduct(zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	shared_ptr<PublishDetails> details = publishMapByID[dataProduct.getDataProductID()];
    if (!details)
        return;

    void* socket = details->socket;

    // Create message & send filter text
    sendStringMessage(socket, filterText, ZMQ_SNDMORE);

    // Serialize data
    zmq_msg_t data;
    zmq_msg_init_size(&data, dataProduct.getSize());
    dataProduct.serializeToArray(zmq_msg_data(&data));

    // Publish data
    zmq_sendmsg(socket, &data, ZMQ_DONTWAIT);

    // Clean up
    zmq_msg_close(&data);

}
} /* namespace gravity */
