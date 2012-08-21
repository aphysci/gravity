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

#include "ComponentLookupRequestPB.pb.h"
#include "ComponentLookupResponsePB.pb.h"
#include "ServiceDirectoryResponsePB.pb.h"
#include "ServiceDirectoryRegistrationPB.pb.h"
#include "ServiceDirectoryUnregistrationPB.pb.h"

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

GravityNode::GravityNode()
{
	// Eventually to be read from a config/properties file
	serviceDirectoryNode.ipAddress = "localhost";
	serviceDirectoryNode.port = 5555;
	serviceDirectoryNode.transport = "tcp";
	serviceDirectoryNode.socket = NULL;
}

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

void GravityNode::sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct)
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
	zmq_msg_init_size(&data, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&data));
	zmq_sendmsg(socket, &data, ZMQ_DONTWAIT);
	zmq_msg_close(&data);
}

GravityReturnCode GravityNode::sendRequestToServiceDirectory(const GravityDataProduct& request,
																GravityDataProduct& response)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Socket to connect to service directory component
	void* socket = NULL;

	int retriesLeft = NETWORK_RETRIES;
	while (retriesLeft && !s_interrupted)
	{
		// Connect to service directory component
		socket = zmq_socket(context, ZMQ_REQ);
		assert(socket);

		stringstream ss;
		ss << serviceDirectoryNode.transport << "://" << serviceDirectoryNode.ipAddress <<
				":" << serviceDirectoryNode.port;
		string serviceDirectoryURL = ss.str();
		zmq_connect(socket, serviceDirectoryURL.c_str());

		// Send registration message to service directory
		sendGravityDataProduct(socket, request);
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

			bool parserSuccess = true;
			try
			{
				response.parseFromArray(zmq_msg_data(&resp), zmq_msg_size(&resp));
			}
			catch (char* s)
			{
				parserSuccess = false;
			}

			// Clean up message
			zmq_msg_close(&resp);

			if (parserSuccess)
			{
				ret = GravityReturnCodes::SUCCESS;
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

	return ret;
}

GravityReturnCode GravityNode::registerDataProduct(string dataProductID, unsigned short networkPort, string transportType)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Build the connection string
	stringstream ss;
	string ipAddr = getIP();
	ss << transportType << "://" << ipAddr << ":" << networkPort;
	string connectionString = ss.str();

	// Create the publish socket
	void* pubSocket = zmq_socket(context, ZMQ_PUB);
	assert(pubSocket);

	// Bind socket to url
	int rc = zmq_bind(pubSocket, connectionString.c_str());
	assert(rc == 0);

	if (!pubSocket || rc != 0)
	{
		ret = GravityReturnCodes::FAILURE;
	}
	else
	{
		// Track dataProductID->socket mapping
	    NetworkNode* node = new NetworkNode;
	    node->ipAddress = ipAddr;
	    node->port = networkPort;
	    node->transport = transportType;
	    node->socket = pubSocket;
		publishMap[dataProductID] = node;
	}

	if (ret == GravityReturnCodes::SUCCESS && !serviceDirectoryNode.ipAddress.empty())
	{
		// Create the object describing the data product to register
		ServiceDirectoryRegistrationPB registration;
		registration.set_id(dataProductID);
		registration.set_url(connectionString);
		registration.set_type(ServiceDirectoryRegistrationPB::DATA);

		// Wrap request in GravityDataProduct
		GravityDataProduct request("DataProductRegistrationRequest");
		request.setFilterText("register");
		request.setData(registration);

		// GravityDataProduct for response
		GravityDataProduct response("DataProductRegistrationResponse");

		// Send request to service directory
		ret = sendRequestToServiceDirectory(request, response);
		if (ret == GravityReturnCodes::SUCCESS)
		{
			ServiceDirectoryResponsePB pb;
			bool parserSuccess = true;
			try
			{
				response.populateMessage(pb);
			}
			catch (char* s)
			{
				parserSuccess = false;
			}

			if (parserSuccess)
			{
				switch (pb.returncode())
				{
					case ServiceDirectoryResponsePB::SUCCESS:
						ret = GravityReturnCodes::SUCCESS;
						break;
					case ServiceDirectoryResponsePB::REGISTRATION_CONFLICT:
						ret = GravityReturnCodes::REGISTRATION_CONFLICT;
						break;
					case ServiceDirectoryResponsePB::DUPLICATE_REGISTRATION:
						ret = GravityReturnCodes::DUPLICATE;
						break;
					case ServiceDirectoryResponsePB::NOT_REGISTERED:
						ret = GravityReturnCodes::LINK_ERROR;
						break;
				}
			}
			else
			{
				ret = GravityReturnCodes::LINK_ERROR;
			}
		}
	}

	return ret;
}

GravityReturnCode GravityNode::unregisterDataProduct(string dataProductID)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    // Track dataProductID->socket mapping
    NetworkNode* node = publishMap[dataProductID];
    if (!node || !node->socket)
    {
        ret = GravityReturnCodes::REGISTRATION_CONFLICT;
    }
    else
    {
        stringstream ss;
        ss << node->transport << "://" << node->ipAddress << ":" << node->port;
        zmq_unbind(node->socket, ss.str().c_str());
        zmq_close(node->socket);
        publishMap[dataProductID] = NULL;
        free(node);

        ServiceDirectoryUnregistrationPB unregistration;
        unregistration.set_id(dataProductID);
        unregistration.set_type(ServiceDirectoryUnregistrationPB::DATA);

        GravityDataProduct request("DataProductUnregistrationRequest");
        request.setFilterText("unregister");
        request.setData(unregistration);

        // GravityDataProduct for response
        GravityDataProduct response("DataProductUnregistrationResponse");

        // Send request to service directory
        ret = sendRequestToServiceDirectory(request, response);

        if (ret == GravityReturnCodes::SUCCESS)
        {
            ServiceDirectoryResponsePB pb;
            bool parserSuccess = true;
            try
            {
                response.populateMessage(pb);
            }
            catch (char* s)
            {
                parserSuccess = false;
            }

            if (parserSuccess)
            {
                switch (pb.returncode())
                {
                    case ServiceDirectoryResponsePB::SUCCESS:
                        ret = GravityReturnCodes::SUCCESS;
                        break;
                    case ServiceDirectoryResponsePB::NOT_REGISTERED:
                        ret = GravityReturnCodes::REGISTRATION_CONFLICT;
                        break;
                }
            }
            else
            {
                ret = GravityReturnCodes::LINK_ERROR;
            }
        }

    }

    return ret;
}

GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter)
{
	// Create the object describing the data product to lookup
	ComponentLookupRequestPB lookup;
	lookup.set_lookupid(dataProductID);
	lookup.set_type(ComponentLookupRequestPB::DATA);

	// Wrap request in GravityDataProduct
	GravityDataProduct request("ComponentLookupRequest");
	request.setFilterText("lookup");
	request.setData(lookup);

	// GravityDataProduct for response
	GravityDataProduct response("ComponentLookupResponse");

	// Send request to service directory
	GravityReturnCode ret = sendRequestToServiceDirectory(request, response);

	if (ret == GravityReturnCodes::SUCCESS)
	{
		ComponentLookupResponsePB pb;
		bool parserSuccess = true;
		try
		{
			response.populateMessage(pb);
		}
		catch (char* s)
		{
			parserSuccess = false;
		}

		if (parserSuccess)
		{
			if (!pb.url().empty())
			{
				// Subscribe to published data product
			}
			else
			{
				ret = GravityReturnCodes::NO_SUCH_DATA_PRODUCT;
			}
		}
		else
		{
			ret = GravityReturnCodes::LINK_ERROR;
		}
	}
	else
	{
		ret = GravityReturnCodes::NO_SERVICE_DIRECTORY;
	}

	return ret;
}

GravityReturnCode GravityNode::subscribe(string connectionURL, string dataProductID,
						    const GravitySubscriber& subscriber, string filter)
{
	// Create the socket
	void* socket = zmq_socket(context, ZMQ_SUB);
	assert(socket);

	// Connect to publisher
	int ret = zmq_connect(socket, connectionURL.c_str());
	assert(ret == 0);

	// Configure filter
	ret = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, filter.c_str(), filter.length());
	assert(ret == 0);

	/*
	zmq_pollitem_t pollItem;
	pollItem.socket = socket;
	pollItem.events = 0;
	pollItem.fd = 0;
	pollItem.revents = ZMQ_POLLIN;

	SubscriptionData subscription;
	subscription.name = subscriptionName;
	subscription.filter = filter;
	subscription.pollItem = pollItem;
	subscriptionMap[subscriptionName] = subscription;
	*/

	return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::unsubscribe(string dataProductID, const GravitySubscriber& subscriber)
{
	return GravityReturnCodes::FAILURE;
}

GravityReturnCode GravityNode::publish(const GravityDataProduct& dataProduct)
{
	string dataProductID = dataProduct.getDataProductID();
	NetworkNode* node = this->publishMap[dataProductID];
	if (!node)
	    return GravityReturnCodes::FAILURE;

	void* socket = node->socket;

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
	string ipAddr = getIP();
	ss << transportType << "://" << ipAddr << ":" << networkPort;
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
	    NetworkNode* node = new NetworkNode;
        node->ipAddress = ipAddr;
        node->port = networkPort;
        node->transport = transportType;
	    node->socket = serverSocket;
		serviceMap[serviceID] = node;
	}

	if (ret == GravityReturnCodes::SUCCESS && !serviceDirectoryNode.ipAddress.empty())
	{
		// Create the object describing the data product to register
		ServiceDirectoryRegistrationPB registration;
		registration.set_id(serviceID);
		registration.set_url(connectionString);
		registration.set_type(ServiceDirectoryRegistrationPB::SERVICE);

		// Wrap request in GravityDataProduct
		GravityDataProduct request("ServiceRegistrationRequest");
		request.setFilterText("register");
		request.setData(registration);

		// GravityDataProduct for response
		GravityDataProduct response("ServiceRegistrationResponse");

		// Send request to service directory
		ret = sendRequestToServiceDirectory(request, response);

		if (ret == GravityReturnCodes::SUCCESS)
		{
			ServiceDirectoryResponsePB pb;
			bool parserSuccess = true;
			try
			{
				response.populateMessage(pb);
			}
			catch (char* s)
			{
				parserSuccess = false;
			}

			if (parserSuccess)
			{
				switch (pb.returncode())
				{
				case ServiceDirectoryResponsePB::SUCCESS:
					ret = GravityReturnCodes::SUCCESS;
					break;
				case ServiceDirectoryResponsePB::REGISTRATION_CONFLICT:
					ret = GravityReturnCodes::REGISTRATION_CONFLICT;
					break;
				case ServiceDirectoryResponsePB::DUPLICATE_REGISTRATION:
					ret = GravityReturnCodes::DUPLICATE;
					break;
				case ServiceDirectoryResponsePB::NOT_REGISTERED:
					ret = GravityReturnCodes::LINK_ERROR;
					break;
				}
			}
			else
			{
				ret = GravityReturnCodes::LINK_ERROR;
			}
		}
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
	string ip = "127.0.0.1";

	if (!serviceDirectoryNode.ipAddress.empty() && serviceDirectoryNode.ipAddress != "localhost")
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
