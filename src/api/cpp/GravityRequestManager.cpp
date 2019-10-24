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
#include "CommUtil.h"
#include <iostream>
#include <sstream>

#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"

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

	gravityResponseSocket = zmq_socket(context, ZMQ_REP);
	zmq_connect(gravityResponseSocket, "inproc://gravity_request_rep");

	// Always have at least the gravity node to poll
	zmq_pollitem_t pollItem;
	pollItem.socket = gravityNodeSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

	// And the rep socket
	zmq_pollitem_t pollItemRep;
	pollItemRep.socket = gravityResponseSocket;
	pollItemRep.events = ZMQ_POLLIN;
	pollItemRep.fd = 0;
	pollItemRep.revents = 0;
	pollItems.push_back(pollItemRep);

	ready();

	// Process forever...
	zmq_msg_t message;
	while (true)
	{
	    // before we poll, determine when the next timeout is (if any)
	    // and handle any expired requests.
	    int64_t nextTimeout = -1;
	    for (map<void*,std::shared_ptr<RequestDetails> >::iterator iter = requestMap.begin(); iter != requestMap.end(); )
	    {
	        int64_t t = iter->second->timeoutTimeMilliseconds;
	        if (t > 0)
	        {
	            int64_t currentTime = getCurrentTime() / 1e3;
	            if (t <= currentTime)
	            {
	                std::shared_ptr<RequestDetails> reqDetails = iter->second;
                    reqDetails->requestor->requestTimeout(reqDetails->serviceID, reqDetails->requestID);
                    void* socket = iter->first;
                    vector<zmq_pollitem_t>::iterator pollItemIter = pollItems.begin() + 2;  // start after internal framework sockets
                    while(pollItemIter != pollItems.end())
                    {
                        if (socket == pollItemIter->socket)
                        {
                            pollItems.erase(pollItemIter);
                            break;
                        }
                    }
                    zmq_close(socket);
                    map<void*,std::shared_ptr<RequestDetails> >::iterator delIter = iter++;
	                requestMap.erase(delIter);
	            }
	            else
	            {
	                if (nextTimeout < 0 || t < nextTimeout)
	                {
	                    nextTimeout = t - currentTime;
	                }
	                iter++;
	            }
	        }
	        else
	        {
	            iter++;
	        }
	    }

		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&pollItems[0], pollItems.size(), nextTimeout); // 0 --> return immediately, -1 --> blocks
		if (rc == -1)
		{
			// Interrupted
			break;
		}

		// Process new subscription requests from the gravity node
		if (pollItems[0].revents & ZMQ_POLLIN)
		{
			// Get new GravityNode request
			string command = readStringMessage(pollItems[0].socket);

			// message from gravity node should be either a request or kill
			if (command == "request")
			{
				processRequest();
			}
			else if (command == "kill")
			{
				break;
			}
			else if (command == "set_service_dir_url")
			{
				serviceDirectoryUrl = readStringMessage(gravityNodeSocket);
			}
			else
			{
				Log::warning("GravityRequestManager received unknown command '%s' from GravityNode", command.c_str());
			}
		}

		if (pollItems[1].revents & ZMQ_POLLIN)
		{
			// Get new GravityNode request
			string command = readStringMessage(pollItems[1].socket);

			if (command == "createFutureResponse")
			{
				createFutureResponse();
			}
			else if (command == "sendFutureResponse")
			{
				sendFutureResponse();
			}
		}

		// init an iterator past the first element so that we can check active requests
		// We can't create the iterator untl here because ProcessRequest may add elements
		// to pollItems which would invalidate the iterator.
        vector<zmq_pollitem_t>::iterator pollItemIter = pollItems.begin() + 2;

        // Check for responses
		std::vector<zmq_pollitem_t> newPollItems;
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
					sendStringMessage(socket, "FUTURE_REQUEST", ZMQ_DONTWAIT);

					// Copy request details from original request socket to future request socket
					requestMap[socket] = requestMap[pollItemIter->socket];					

					// Add new poll item once we're done with this loop
					zmq_pollitem_t pollItem;
					pollItem.socket = socket;
					pollItem.events = ZMQ_POLLIN;
					pollItem.fd = 0;
					pollItem.revents = 0;
					newPollItems.push_back(pollItem);					
				}
				else
				{
					// Get request details
					std::shared_ptr<RequestDetails> reqDetails = requestMap[pollItemIter->socket];

					// Verify the service provider
					if (response.getRegistrationTime() != reqDetails->registrationTime)
					{
						Log::critical("Received service (%s) response from invalid service [%u != %u]. Aborting.",
							reqDetails->serviceID.c_str(), response.getRegistrationTime(), reqDetails->registrationTime);

						// Send notification of stale data to ServiceDirectory
						notifyServiceDirectoryOfStaleEntry(reqDetails->serviceID, reqDetails->url, reqDetails->registrationTime);
					}
					else
					{
						// Deliver to requestor
						Log::trace("GravityRequestManager: call requestFilled()");
						reqDetails->requestor->requestFilled(reqDetails->serviceID, reqDetails->requestID, response);
					}
				}

				zmq_close(pollItemIter->socket);
				requestMap.erase(pollItemIter->socket);	
				pollItemIter = pollItems.erase(pollItemIter);
			}
			else
			{
			    pollItemIter++;
			}
		}

		pollItems.insert(pollItems.end(), newPollItems.begin(), newPollItems.end());
	}

	// Clean up all our open sockets
	for (map<void*,std::shared_ptr<RequestDetails> >::iterator iter = requestMap.begin(); iter != requestMap.end(); iter++)
	{
		void* socket = iter->first;
		zmq_close(socket);
	}
	zmq_close(gravityNodeSocket);
	zmq_close(gravityResponseSocket);
}

void GravityRequestManager::notifyServiceDirectoryOfStaleEntry(string serviceId, string url, uint32_t regTime)
{
	Log::debug("Notifying ServiceDirectory of stale service: [%s @ %s]", serviceId.c_str(), url.c_str());
	ServiceDirectoryUnregistrationPB unregistration;
	unregistration.set_id(serviceId);
	unregistration.set_url(url);
	unregistration.set_registration_time(regTime);
	unregistration.set_type(ServiceDirectoryUnregistrationPB::SERVICE);

	GravityDataProduct request("UnregistrationRequest");
	request.setData(unregistration);

	void* socket = zmq_socket(context, ZMQ_REQ); // Socket to connect to service provider
	zmq_connect(socket, serviceDirectoryUrl.c_str());
	int linger = -1;
	zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

	// Send message to service provider
	sendGravityDataProduct(socket, request, ZMQ_DONTWAIT);

	zmq_close(socket);
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

void GravityRequestManager::sendFutureResponse()
{
	int ret = GravityReturnCodes::SUCCESS;
	string url = readStringMessage(gravityResponseSocket);

	zmq_msg_t message;
	zmq_msg_init(&message);
    zmq_recvmsg(gravityResponseSocket, &message, 0);
    // Create new GravityDataProduct from the incoming message
	GravityDataProduct response(zmq_msg_data(&message), zmq_msg_size(&message));
    // Clean up message
    zmq_msg_close(&message);

	if (futureResponseUrlToSocketMap.find(url) == futureResponseUrlToSocketMap.end())
	{
		Log::warning("Received a future response that is not associated with a REP url ('%s')", url.c_str());
		ret = GravityReturnCodes::FAILURE;
	}
	else
	{
		void* responseSocket = futureResponseUrlToSocketMap[url];
		
		// Poll socket for pending request
		zmq_pollitem_t pollItems[] = {{responseSocket, 0, ZMQ_POLLIN, 0}};
		int rc = zmq_poll(pollItems, 1, 2000); // 2 second timeout - this really should be waiting for us
		if (rc == -1)
		{
			ret = GravityReturnCodes::INTERRUPTED;
		}
		else if (rc == 0)
		{
			ret = GravityReturnCodes::REQUEST_TIMEOUT;
		}
		else if (pollItems[0].revents & ZMQ_POLLIN)
		{
			// Don't really care about the specifics of the request, just read and discard
			zmq_msg_init(&message);
			zmq_recvmsg(responseSocket, &message, 0);
			zmq_msg_close(&message);

			sendGravityDataProduct(responseSocket, response, ZMQ_DONTWAIT);

			// This brief sleep seems to be required to avoid zeromq bug
			gravity::sleep(100);
		}

		// Return result
		sendIntMessage(gravityResponseSocket, ret, ZMQ_DONTWAIT);

		// Clean up the future response socket
		zmq_close(responseSocket);
		futureResponseUrlToSocketMap.erase(url);		
	}
}

void GravityRequestManager::createFutureResponse()
{
	// Read our IP address
	string ip = readStringMessage(gravityResponseSocket);

	// Read port range
	int minPort = readIntMessage(gravityResponseSocket);
	int maxPort = readIntMessage(gravityResponseSocket);

	// Create the response socket
	void* responseSocket = zmq_socket(context, ZMQ_REP);

	// Bind socket and create URL
	string url = "";
	int port = bindFirstAvailablePort(responseSocket, ip, minPort, maxPort);
    if (port < 0)
    {
        Log::critical("Could not find available port for FutureResponse");
        zmq_close(responseSocket);
    }
	else
	{
		stringstream ss;
		ss << "tcp://" << ip << ":" << port;
		url = ss.str();

		futureResponseUrlToSocketMap[url] = responseSocket;
	}

	Log::trace("GravityRequestManager::createFutureResponse(): URL = '%s'", url.c_str());

	sendStringMessage(gravityResponseSocket, url, ZMQ_DONTWAIT);
}

void GravityRequestManager::processRequest()
{
	// Read the service id for this request
	string serviceID = readStringMessage(gravityNodeSocket);

	// Read the service url
	string url = readStringMessage(gravityNodeSocket);

	// Read the request ID
	string requestID = readStringMessage(gravityNodeSocket);

	int timeout_milliseconds = readIntMessage(gravityNodeSocket);
	int64_t timeoutTimeMilliseconds = -1;
	// calculate an actual time in milliseconds
	if (timeout_milliseconds > 0)
	{
	    timeoutTimeMilliseconds = getCurrentTime() / 1e3 + timeout_milliseconds;
	}

	uint32_t regTime = readUint32Message(gravityNodeSocket);

	// Read the data product
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityDataProduct dataProduct(zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);
	dataProduct.setRegistrationTime(regTime);

	// Read the data product
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket, &msg, -1);
	GravityRequestor* requestor;
	memcpy(&requestor, zmq_msg_data(&msg), zmq_msg_size(&msg));
	zmq_msg_close(&msg);

	std::shared_ptr<RequestDetails> reqDetails;

	// Create the request socket
	void* reqSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(reqSocket, url.c_str());
    int linger = 0;
    zmq_setsockopt(reqSocket, ZMQ_LINGER, &linger, sizeof(linger));

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
	reqDetails->requestor = requestor;
	reqDetails->timeoutTimeMilliseconds = timeoutTimeMilliseconds;
	reqDetails->registrationTime = regTime;
	reqDetails->url = url;
	
	requestMap[reqSocket] = reqDetails;
}

} /* namespace gravity */
