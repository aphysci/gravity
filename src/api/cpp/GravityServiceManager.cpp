/*
 * GravityServiceManager.cpp
 *
 *  Created on: Aug 30, 2012
 *      Author: Chris Brundick
 */

#include "GravityServiceManager.h"
#include <iostream>

namespace gravity {

using namespace std;

GravityServiceManager::GravityServiceManager(void* context)
{
	this->context = context;
}

GravityServiceManager::~GravityServiceManager() {}

void GravityServiceManager::start()
{
	// Messages
	zmq_msg_t filter, message;

	// Set up the inproc socket to subscribe to request messages from the GravityNode
	gravityNodeSocket = zmq_socket(context, ZMQ_SUB);
	zmq_connect(gravityNodeSocket, "inproc://gravity_service_manager");
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

			// message from gravity node should be either a request or kill
			if (command == "register")
			{
				addService();
			}
			if (command == "unregister")
			{
				removeService();
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

		// Check for service requests
		for (unsigned int i = 1; i < pollItems.size(); i++)
		{
			if (pollItems[i].revents && ZMQ_POLLIN)
			{
				// Read response data product from socket
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

				shared_ptr<ServiceDetails> serviceDetails = serviceMapBySocket[pollItems[i].socket];
				shared_ptr<GravityDataProduct> response = serviceDetails->server->request(dataProduct);

				sendGravityDataProduct(pollItems[i].socket, *response);
			}
		}
	}

	// Clean up all our open sockets
	for (map<void*,shared_ptr<ServiceDetails> >::iterator iter = serviceMapBySocket.begin(); iter != serviceMapBySocket.end(); iter++)
	{
		void* socket = iter->first;
		zmq_close(socket);
	}

	serviceMapBySocket.clear();
	serviceMapByServiceID.clear();

	zmq_close(gravityNodeSocket);
}

void GravityServiceManager::ready()
{
	// Create the request socket
	void* initSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(initSocket, "inproc://gravity_init");

	// Send request to service provider
	sendStringMessage(initSocket, "GravityServiceManager", ZMQ_DONTWAIT);

	zmq_close(initSocket);
}

void GravityServiceManager::sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct)
{
    // Send raw filter text as first part of message
	sendStringMessage(socket, dataProduct.getFilterText(), ZMQ_SNDMORE);

    // Send data product
    zmq_msg_t data;
    zmq_msg_init_size(&data, dataProduct.getSize());
    dataProduct.serializeToArray(zmq_msg_data(&data));
    zmq_sendmsg(socket, &data, ZMQ_DONTWAIT);
    zmq_msg_close(&data);
}

string GravityServiceManager::readStringMessage()
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

void GravityServiceManager::sendStringMessage(void* socket, string str, int flags)
{
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());
	zmq_sendmsg(socket, &msg, flags);
	zmq_msg_close(&msg);
}

void GravityServiceManager::addService()
{
	// Read the serive id for this request
	string serviceID = readStringMessage();

	// Read the service url
	string url = readStringMessage();

	// Read the server pointer
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityServiceProvider* server;
	memcpy(&server, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	shared_ptr<ServiceDetails> serviceDetails;

	// Create the response socket
	void* serverSocket = zmq_socket(context, ZMQ_REP);
	zmq_bind(serverSocket, url.c_str());

	// Create poll item for response to this request
	zmq_pollitem_t pollItem;
	pollItem.socket = serverSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

	// Create request details
	serviceDetails.reset(new ServiceDetails());
	serviceDetails->serviceID = serviceID;
	serviceDetails->url = url;
	serviceDetails->pollItem = pollItem;
	serviceDetails->server = server;

	serviceMapBySocket[serverSocket] = serviceDetails;
	serviceMapByServiceID[serviceID] = serviceDetails;
}

void GravityServiceManager::removeService()
{
	// Read the service id for this request
	string serviceID = readStringMessage();

	// If service ID exists, clean up and remove socket. Otherwise, likely a duplicate unregister request
	if (serviceMapByServiceID.count(serviceID))
	{
		shared_ptr<ServiceDetails> serviceDetails = serviceMapByServiceID[serviceID];
		void* socket = serviceDetails->pollItem.socket;
		serviceMapBySocket.erase(socket);
		serviceMapByServiceID.erase(serviceID);
		zmq_unbind(socket, serviceDetails->url.c_str());
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

} /* namespace gravity */
