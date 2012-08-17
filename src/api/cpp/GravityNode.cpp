/*
 * GravityNode.cpp
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#include "GravityNode.h"

#include <zmq.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <signal.h>
#include <tr1/memory>

#include "ServiceDirectoryResponsePB.pb.h"
#include "ServiceDirectoryRegistrationPB.pb.h"

static int s_interrupted = 0;
static void s_signal_handler(int signal_value)
{
	s_interrupted = 1;
}
void s_catch_signals()
{
	struct sigaction action;
	action.sa_handler = s_signal_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}

namespace gravity
{

using namespace std::tr1;

GravityNode::GravityNode() {}

GravityNode::~GravityNode()
{
	// Clean up the zmq context object
	zmq_term(context);
}

GravityReturnCode GravityNode::init()
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Setup zmq context
	context = zmq_init(1);
	if (!context)
	{
		ret = GravityReturnCodes::FAILURE;
	}

	// Configure to trap Ctrl-C (SIGINT) and SIGTERM signals
	s_catch_signals();

	return ret;
}

GravityReturnCode GravityNode::sendGravityDataProduct(void* socket, GravityDataProduct& dataProduct)
{
	// Send raw filter text as first part of message
	string filterText = dataProduct.getFilterText();
	zmq_msg_t filt;
	zmq_msg_init_size(&filt, filterText.length());
	memcpy(zmq_msg_data(&filt), filterText.c_str(), filterText.length());
	zmq_sendmsg(socket, &filt, ZMQ_SNDMORE);
	zmq_msg_close(&filt);

	// Send data product
	zmq_msg_t data;
	zmq_msg_init_size(&data, dataProduct.getDataSize());
	dataProduct.getData(zmq_msg_data(&data), dataProduct.getDataSize());
	zmq_sendmsg(socket, &data, ZMQ_DONTWAIT);
	zmq_msg_close(&data);

	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::registerWithServiceDirectory(const ServiceDirectoryRegistrationPB& registration)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Socket to connect to service directory component
	void* socket = NULL;

	shared_ptr<GravityDataProduct> gdp(new GravityDataProduct("DataProductRegistrationRequest"));
	gdp->setFilterText("register");
	gdp->setData(registration);

	int retriesLeft = NETWORK_RETRIES;
	while (retriesLeft && !s_interrupted)
	{
		// Connect to service directory component
		socket = zmq_socket(context, ZMQ_REQ);
		zmq_connect(socket, registration.url().c_str());

		// Send registration message to service directory
		sendGravityDataProduct(socket, *gdp);
		retriesLeft--;

		// Poll socket for reply with a timeout
		zmq_pollitem_t items[] = {{socket, 0, ZMQ_POLLIN, 0}};
		int rc = zmq_poll(items, 1, NETWORK_TIMEOUT);
		if (rc == -1)
		{
			// Interrupted
			ret = GravityReturnCodes::INTERRUPTED;
			break;
		}

		// Process the directory service response
		if (items[0].revents & ZMQ_POLLIN)
		{
			// Get service directory response
			zmq_msg_t resp;
			zmq_msg_init(&resp);
			zmq_recvmsg(socket, &resp, 0);
			//string responseString((char*)zmq_msg_data(&resp), zmq_msg_size(&resp));

			// Parse response
			bool parserSuccess = true;
			shared_ptr<ServiceDirectoryResponsePB> sdResponse(new ServiceDirectoryResponsePB());
			try
			{
				sdResponse->ParseFromArray(zmq_msg_data(&resp), zmq_msg_size(&resp));
			}
			catch (char* s)
			{
				parserSuccess = false;
			}

			// Clean up message
			zmq_msg_close(&resp);

			if (parserSuccess)
			{
				if (sdResponse->returncode() == sdResponse->SUCCESS)
				{
					// Successfully registered data product
					ret = GravityReturnCodes::SUCCESS;
				}
				else if (sdResponse->returncode() == sdResponse->REGISTRATION_CONFLICT)
				{
					ret = GravityReturnCodes::REGISTRATION_CONFLICT;
				}
				else if (sdResponse->returncode() == sdResponse->DUPLICATE_REGISTRATION)
				{
					ret = GravityReturnCodes::DUPLICATE;
				}
				retriesLeft = 0;
				break;
			}
			else
			{
				// Bad response.
				ret = GravityReturnCodes::LINK_ERROR;
			}
		}
		else
		{
			ret = GravityReturnCodes::NO_SERVICE_DIRECTORY;
		}

		// Close socket
		zmq_close(socket);
	}

	// clean up the connection
	zmq_close(socket);

	return ret;
}

GravityReturnCode GravityNode::registerDataProduct(string dataProductID, unsigned short networkPort, string transportType)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Build the connection string
	stringstream ss;
	ss << transportType << "://" << getIP() << ":" << networkPort;
	string connectionString = ss.str();

	// Create the publish socket
	void* pubSocket = zmq_socket(context, ZMQ_PUB);

	// Bind socket to url
	int rc = zmq_bind(pubSocket, connectionString.c_str());

	if (!pubSocket || rc == 0)
	{
		ret = GravityReturnCodes::FAILURE;
	}
	else
	{
		// Track dataProductID->socket mapping
		publishMap[dataProductID] = pubSocket;
	}

	if (ret == GravityReturnCodes::SUCCESS && serviceDirectoryNode.ipAddress.empty())
	{
		// Create the object describing the data product to register
		shared_ptr<ServiceDirectoryRegistrationPB> registration(new ServiceDirectoryRegistrationPB());
		registration->set_id(dataProductID);
		registration->set_url(connectionString);
		registration->set_type(ServiceDirectoryRegistrationPB::DATA);

		// Send registration request
		ret = registerWithServiceDirectory(*registration);
	}

	return ret;
}

GravityReturnCode GravityNode::unregisterDataProduct(string dataProductID)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::subscribe(string connectionURL, string dataProductID,
						    const GravitySubscriber& subscriber, string filter)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::unsubscribe(string dataProductID, const GravitySubscriber& subscriber)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::publish(const GravityDataProduct& dataProduct)
{
	string dataProductID = dataProduct.getDataProductID();
	void* socket = this->publishMap[dataProductID];

	// Create message & send filter text
	zmq_msg_t filt;
	string filterText = dataProduct.getFilterText();
	zmq_msg_init_size(&filt, filterText.length());
	memcpy(zmq_msg_data(&filt), filterText.c_str(), filterText.length());
	zmq_sendmsg(socket, &filt, ZMQ_SNDMORE);

	// Serialize data
	zmq_msg_t data;
	zmq_msg_init_size(&data, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&data));

	// Publish data
	zmq_sendmsg(socket, &data, ZMQ_DONTWAIT);

	// Clean up
	zmq_msg_close(&data);
	zmq_msg_close(&filt);

	return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::request(string serviceID, const GravityDataProduct& dataProduct,
						  const GravityRequestor& requestor, string requestID)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
						  const GravityRequestor& requestor, string requestID)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::registerService(string serviceID, unsigned short networkPort,
											   string transportType, const GravityServiceProvider& server)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Build the connection string
	stringstream ss;
	ss << transportType << "://" << getIP() << ":" << networkPort;
	string connectionString = ss.str();

	// Create the server socket
	void* serverSocket = zmq_socket(context, ZMQ_REP);

	// Bind socket to url
	int rc = zmq_bind(serverSocket, connectionString.c_str());

	if (!serverSocket || rc == 0)
	{
		ret = GravityReturnCodes::FAILURE;
	}
	else
	{
		// Track serviceID->socket mapping
		serviceMap[serviceID] = serverSocket;
	}

	if (ret == GravityReturnCodes::SUCCESS && !serviceDirectoryNode.ipAddress.empty())
	{
		// Create the object describing the data product to register
		shared_ptr<ServiceDirectoryRegistrationPB> registration(new ServiceDirectoryRegistrationPB());
		registration->set_id(serviceID);
		registration->set_url(connectionString);
		registration->set_type(ServiceDirectoryRegistrationPB::SERVICE);

		// Send registration request
		ret = registerWithServiceDirectory(*registration);
	}

	return ret;
}

GravityReturnCode GravityNode::unregisterService(string serviceID, const GravityServiceProvider& server)
{
	return GravityReturnCodes::FAILURE;
}

uint64_t GravityNode::getCurrentTime()
{
	timespec ts;
	clock_gettime(0, &ts);
	return (uint64_t)ts.tv_sec * 1000000LL + (uint64_t)ts.tv_nsec / 1000LL;
}

string GravityNode::getIP()
{
	string ip = "localhost";

	if (!serviceDirectoryNode.ipAddress.empty())
	{
		int buflen = 16;
		char* buffer = (char*)malloc(buflen);

		int sock = socket(AF_INET, SOCK_DGRAM, 0);
		assert(sock != -1);

		const char* otherIP = serviceDirectoryNode.ipAddress.c_str();
		uint16_t otherPort = serviceDirectoryNode.port;

		struct sockaddr_in serv;
		memset(&serv, 0, sizeof(serv));
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = inet_addr(otherIP);
		serv.sin_port = htons(otherPort);

		int err = connect(sock, (const sockaddr*)&serv, sizeof(serv));
		assert(err != -1);

		sockaddr_in name;
		socklen_t namelen = sizeof(name);
		err = getsockname(sock, (sockaddr*)&name, &namelen);
		assert(err != -1);

		const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
		assert(p);

		close(sock);

		ip.assign(buffer);
	}
	return ip;
}

} /* namespace gravity */
