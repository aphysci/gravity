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
 * GravityRequestManager.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: Chris Brundick
 */

#include "GravityRequestManager.h"
#include "GravityLogger.h"
#include <iostream>

namespace gravity
{

using namespace std;

GravityRequestManager::GravityRequestManager(void* context)
{
	// This is the zmq context used to the comms socket
	this->context = context;
}

GravityRequestManager::~GravityRequestManager() {}

void GravityRequestManager::start()
{
	// Set up the inproc socket to subscribe to request messages from the GravityNode
	gravityNodeSocket = zmq_socket(context, ZMQ_SUB);
	zmq_connect(gravityNodeSocket, "inproc://gravity_request_manager");
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
	zmq_msg_t message;
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
			if (command == "request")
			{
				processRequest();
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

		// init an iterator past the first element so that we can check active requests
		// We can't create the iterator untl here because ProcessRequest may add elements
		// to pollItems which would invalidate the iterator.
        vector<zmq_pollitem_t>::iterator pollItemIter = pollItems.begin() + 1;

        // Check for responses
		while(pollItemIter != pollItems.end())
		{
			if (pollItemIter->revents & ZMQ_POLLIN)
			{
				zmq_msg_init(&message);
				zmq_recvmsg(pollItemIter->socket, &message, 0);
				// Create new GravityDataProduct from the incoming message
				GravityDataProduct response(zmq_msg_data(&message), zmq_msg_size(&message));
				// Clean up message
				zmq_msg_close(&message);

				if (response.isFutureResponse())
				{					
					Log::trace("Received a future response placeholder (url = %s)", response.getFutureSocketUrl().c_str());
					// Create REQ socket to interact with future REP socket
					void* socket = zmq_socket(context, ZMQ_REQ);
					zmq_connect(socket, response.getFutureSocketUrl().c_str());					

					// Send request to future REP socket
					Log::trace("Sending empty request on future response socket");
					sendStringMessage(socket, "", ZMQ_DONTWAIT);

					// Replace polling info for original socket with future socket details
					requestMap[socket] = requestMap[pollItemIter->socket];
					requestMap.erase(pollItemIter->socket);
					pollItemIter->socket = socket;
					pollItemIter->revents = 0;
				}
				else
				{
					// Deliver to requestor
					shared_ptr<RequestDetails> reqDetails = requestMap[pollItemIter->socket];
					Log::trace("GravityRequestManager: call requestFilled()");
					reqDetails->requestor->requestFilled(reqDetails->serviceID, reqDetails->requestID, response);
					zmq_close(pollItemIter->socket);
					requestMap.erase(pollItemIter->socket);
					pollItemIter = pollItems.erase(pollItemIter);
				}
			}
			else
			{
			    pollItemIter++;
			}
		}
	}

	// Clean up all our open sockets
	for (map<void*,shared_ptr<RequestDetails> >::iterator iter = requestMap.begin(); iter != requestMap.end(); iter++)
	{
		void* socket = iter->first;
		zmq_close(socket);
	}
	zmq_close(gravityNodeSocket);
}

void GravityRequestManager::ready()
{
	// Create the request socket
	void* initSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(initSocket, "inproc://gravity_init");

	// Send request to service provider
	sendStringMessage(initSocket, "GravityRequestManager", ZMQ_DONTWAIT);

	zmq_close(initSocket);
}

string GravityRequestManager::readStringMessage()
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
	free(s);
	zmq_msg_close(&msg);

	return str;
}

void GravityRequestManager::sendStringMessage(void* socket, string str, int flags)
{
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());
	zmq_sendmsg(socket, &msg, flags);
	zmq_msg_close(&msg);
}

void GravityRequestManager::processRequest()
{
	// Read the service id for this request
	string serviceID = readStringMessage();

	// Read the service url
	string url = readStringMessage();

	// Read the request ID
	string requestID = readStringMessage();

	// Read the data product
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityDataProduct dataProduct(zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	// Read the data product
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityRequestor* requestor;
	memcpy(&requestor, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	shared_ptr<RequestDetails> reqDetails;

	// Create the request socket
	void* reqSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(reqSocket, url.c_str());

	// Send data product
	zmq_msg_t data;
	zmq_msg_init_size(&data, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&data));
	zmq_sendmsg(reqSocket, &data, ZMQ_DONTWAIT);
	zmq_msg_close(&data);

	// Create poll item for response to this request
	zmq_pollitem_t pollItem;
	pollItem.socket = reqSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

	// Create request details
	reqDetails.reset(new RequestDetails());
	reqDetails->serviceID = serviceID;
	reqDetails->requestID = requestID;
	reqDetails->pollItem = pollItem;
	reqDetails->requestor = requestor;

	requestMap[reqSocket] = reqDetails;
}

} /* namespace gravity */
