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
 * GravityServiceManager.cpp
 *
 *  Created on: Aug 30, 2012
 *      Author: Chris Brundick
 */

#include "GravityServiceManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include <sstream>

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
	zmq_msg_t message;

	// Set up the inproc socket to listen for to requests messages from the GravityNode
	gravityNodeSocket = zmq_socket(context, ZMQ_REP);
	zmq_bind(gravityNodeSocket, SERVICE_MGR_URL);

	// Always have at least the gravity node to poll
	zmq_pollitem_t pollItem;
	pollItem.socket = gravityNodeSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

	void* configureSocket=zmq_socket(context,ZMQ_SUB);
	zmq_connect(configureSocket,"inproc://gravity_service_manager_configure");
	zmq_setsockopt(configureSocket, ZMQ_SUBSCRIBE, NULL, 0);

	// Always have at least the gravity node to poll
	zmq_pollitem_t configItem;
	configItem.socket = configureSocket;
	configItem.events = ZMQ_POLLIN;
	configItem.fd = 0;
	configItem.revents = 0;

	ready();

	string domain="";
	string componentID="";

	//receive Gravity node parameters
	bool configured = false;
	while(!configured)
	{
		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&configItem, 1, -1); // 0 --> return immediately, -1 --> blocks
		if (rc == -1)
		{
			// Interrupted
			break;
		}

		// Process new subscription requests from the gravity node
		if (configItem.revents & ZMQ_POLLIN)
		{
			string command = readStringMessage(configureSocket);
			if(command == "configure")
			{
				//receive Gravity node details
				domain.assign(readStringMessage(configureSocket));
				componentID.assign(readStringMessage(configureSocket));

				configured=true;
			}
		}
	}

	zmq_close(configureSocket);

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

			// message from gravity node should be either a request or kill
			if (command == "register")
			{
				addService();
			}
			else if (command == "unregister")
			{
				removeService();
			}
			else if (command == "kill")
			{
				break;
			}
			else
			{
				Log::warning("GravityServiceManager received unknown command '%s' from GravityNode", command.c_str());
			}
		}

		// Check for service requests
		for (unsigned int i = 1; i < pollItems.size(); i++)
		{
			if (pollItems[i].revents & ZMQ_POLLIN)
			{
				Log::trace("Received a service request on %s", serviceMapBySocket[pollItems[i].socket]->url.c_str());
				// Read response data product from socket
				zmq_msg_init(&message);
				zmq_recvmsg(pollItems[i].socket, &message, 0);
				// Create new GravityDataProduct from the incoming message
				GravityDataProduct dataProduct(zmq_msg_data(&message), zmq_msg_size(&message));
				// Clean up message
				zmq_msg_close(&message);
				
				std::shared_ptr<GravityDataProduct> response;
				std::shared_ptr<ServiceDetails> serviceDetails = serviceMapBySocket[pollItems[i].socket];				
				if (dataProduct.getRegistrationTime() != 0 && dataProduct.getRegistrationTime() != serviceRegistrationTimeMap[serviceDetails->serviceID])
				{
					// Invalid request - likely due to a stale service directory entry
					response = std::shared_ptr<GravityDataProduct>(new GravityDataProduct(serviceDetails->serviceID));
				}
				else
				{
					Log::trace("Sending request to provider for serviceID = '%s'", serviceDetails->serviceID.c_str());
					response = serviceDetails->server->request(serviceDetails->serviceID, dataProduct);
				}

				response->setComponentId(componentID);
				response->setDomain(domain);
				response->setRegistrationTime(serviceRegistrationTimeMap[serviceDetails->serviceID]);

				sendGravityDataProduct(pollItems[i].socket, *response, ZMQ_DONTWAIT);
			}
		}
	}

	// Clean up all our open sockets
	for (map<void*,std::shared_ptr<ServiceDetails> >::iterator iter = serviceMapBySocket.begin(); iter != serviceMapBySocket.end(); iter++)
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

void GravityServiceManager::addService()
{
	// Read the serive id for this request
	string serviceID = readStringMessage(gravityNodeSocket);

    // Read the publish transport type
    string transportType = readStringMessage(gravityNodeSocket);

    int minPort, maxPort;
    if(transportType == "tcp")
    {
        minPort = readIntMessage(gravityNodeSocket);
        maxPort = readIntMessage(gravityNodeSocket);
    }

    // Read the publish transport type
    string endpoint = readStringMessage(gravityNodeSocket);

	// Read the registration time
	uint32_t registrationTime = readUint32Message(gravityNodeSocket);

	// Read the server pointer
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityServiceProvider* server;
	memcpy(&server, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	std::shared_ptr<ServiceDetails> serviceDetails;

	// Create the response socket
	void* serverSocket = zmq_socket(context, ZMQ_REP);
    string connectionURL;
    if(transportType == "tcp")
    {
        int port = bindFirstAvailablePort(serverSocket, endpoint, minPort, maxPort);
        if (port < 0)
        {
            Log::critical("Could not find available port for %s in range [%d,%d]", serviceID.c_str(), minPort, maxPort);
            zmq_close(serverSocket);
            sendStringMessage(gravityNodeSocket, "", ZMQ_DONTWAIT);
            return;
        }
        stringstream ss;
        ss << transportType << "://" << endpoint << ":" << port;
        connectionURL = ss.str();
    }
    else
    {
        stringstream ss;
        ss << transportType << "://" << endpoint;
        connectionURL = ss.str();
        int rc = zmq_bind(serverSocket, ss.str().c_str());
        if (rc < 0)
        {
            Log::critical("Could not bind address %s", connectionURL.c_str());
            zmq_close(serverSocket);
            sendStringMessage(gravityNodeSocket, "", ZMQ_DONTWAIT);
            return;
        }
    }

    sendStringMessage(gravityNodeSocket, connectionURL, ZMQ_DONTWAIT);

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
	serviceDetails->url = connectionURL;
	serviceDetails->pollItem = pollItem;
	serviceDetails->server = server;

	serviceMapBySocket[serverSocket] = serviceDetails;
	serviceMapByServiceID[serviceID] = serviceDetails;
	serviceRegistrationTimeMap[serviceID] = registrationTime;
}

void GravityServiceManager::removeService()
{
	// Read the service id for this request
	string serviceID = readStringMessage(gravityNodeSocket);

	// If service ID exists, clean up and remove socket. Otherwise, likely a duplicate unregister request
	if (serviceMapByServiceID.count(serviceID))
	{
	    std::shared_ptr<ServiceDetails> serviceDetails = serviceMapByServiceID[serviceID];
		void* socket = serviceDetails->pollItem.socket;
		serviceMapBySocket.erase(socket);
		serviceMapByServiceID.erase(serviceID);
		serviceRegistrationTimeMap.erase(serviceID);
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

	sendStringMessage(gravityNodeSocket, "OK", ZMQ_DONTWAIT);
}

} /* namespace gravity */
