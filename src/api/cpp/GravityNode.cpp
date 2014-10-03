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
 * GravityNode.cpp
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#include <zmq.h>
#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <boost/assign.hpp>
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#endif
#include <sstream>
#include <signal.h>
#ifndef __GNUC__
#include <memory>
#else
#include <tr1/memory>
#endif

#include "GravityMetricsUtil.h"
#include "GravityMetricsManager.h"
#include "GravitySubscriptionManager.h"
#include "GravityPublishManager.h"
#include "GravityRequestManager.h"
#include "GravityServiceManager.h"
#include "GravityHeartbeatListener.h"
#include "GravityHeartbeat.h"
#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include "CommUtil.h"

#include "protobuf/ComponentLookupRequestPB.pb.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ComponentServiceLookupResponsePB.pb.h"
#include "protobuf/ServiceDirectoryResponsePB.pb.h"
#include "protobuf/ServiceDirectoryRegistrationPB.pb.h"
#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"

#include "GravityNode.h" //Needs to be last on Windows so it is included after nb30.h for the DUPLICATE definition.

static void* startSubscriptionManager(void* context)
{
	// Create and start the GravitySubscriptionManager
	gravity::GravitySubscriptionManager subManager(context);
	subManager.start();

	return NULL;
}

static void* startPublishManager(void* context)
{
	// Create and start the GravitySubscriptionManager
	gravity::GravityPublishManager pubManager(context);
	pubManager.start();

	return NULL;
}

static void* startRequestManager(void* context)
{
	// Create and start the GravityRequestManager
	gravity::GravityRequestManager reqManager(context);
	reqManager.start();

	return NULL;
}

static void* startServiceManager(void* context)
{
	// Create and start the GravityServiceManager
	gravity::GravityServiceManager serviceManager(context);
	serviceManager.start();

	return NULL;
}

static void* startMetricsManager(void* context)
{
    // Create and start the GravityMetricsManager
    gravity::GravityMetricsManager metricsManager(context);
    metricsManager.start();

    return NULL;
}

static int s_interrupted = 0;
static void (*previousHandlerAbrt)(int); //Function Pointer
static void (*previousHandlerInt)(int); //Function Pointer
void s_restore_signals()
{
	signal(SIGABRT, previousHandlerAbrt);
	signal(SIGINT, previousHandlerInt);
}

static void s_signal_handler(int signal_value)
{
    s_interrupted = signal_value;
    s_restore_signals();
}

void s_catch_signals()
{
	previousHandlerAbrt = signal(SIGABRT, s_signal_handler);
	previousHandlerInt = signal(SIGINT, s_signal_handler);
}


namespace gravity
{

//Forward Declarations that we don't want publicly visible (Need to be in gravity namespace).
bool IsValidFilename(const std::string filename);
int StringToInt(std::string str, int default_value);
double StringToDouble(std::string str, double default_value);
void bindHeartbeatSocket(void* context);
void closeHeartbeatSocket();
void* Heartbeat(void* thread_context);

using namespace std;
using namespace std::tr1;

GravityNode::GravityNode()
{
    // Eventually to be read from a config/properties file
    serviceDirectoryNode.port = 5555;
    serviceDirectoryNode.transport = "tcp";
    serviceDirectoryNode.socket = NULL;

    //Initialize this guy so we can know whether the heartbeat thread has started.
    hbSocket = NULL;

    // Default to no metrics
    metricsEnabled = false;
}

GravityNode::~GravityNode()
{
    // If metrics are enabled, we need to unregister our metrics data product
    if (metricsEnabled)
    {
        unregisterDataProduct(GRAVITY_METRICS_DATA_PRODUCT_ID);
    }

    closeHeartbeatSocket();

	// Close the inproc sockets
	sendStringMessage(subscriptionManagerSocket, "kill", ZMQ_DONTWAIT);
	zmq_close(subscriptionManagerSocket);

	sendStringMessage(requestManagerSocket, "kill", ZMQ_DONTWAIT);
	zmq_close(requestManagerSocket);

    sendStringMessage(publishManagerPublishSocket, "kill", ZMQ_DONTWAIT);
    zmq_close(publishManagerPublishSocket);
    zmq_close(publishManagerRequestSocket);

    sendStringMessage(serviceManagerSocket, "kill", ZMQ_DONTWAIT);
    zmq_close(serviceManagerSocket);

    sendStringMessage(metricsManagerSocket, "kill", ZMQ_DONTWAIT);
    zmq_close(metricsManagerSocket);

    // Clean up the zmq context object
    zmq_term(context);

    delete parser;
}

GravityReturnCode GravityNode::init(std::string componentID)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;
	this->componentID = componentID;

    // Setup zmq context
    context = zmq_init(1);
    if (!context)
    {
        ret = GravityReturnCodes::FAILURE;
    }

    void* initSocket = zmq_socket(context, ZMQ_REP);
    zmq_bind(initSocket, "inproc://gravity_init");
    bindHeartbeatSocket(context);

    // Setup up communication channel to subscription manager
    subscriptionManagerSocket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(subscriptionManagerSocket, "inproc://gravity_subscription_manager");

    // Setup the metrics control communication channel
    metricsManagerSocket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(metricsManagerSocket, GRAVITY_METRICS_CONTROL);

    // Setup the subscription manager
    pthread_create(&subscriptionManagerThread, NULL, startSubscriptionManager, context);

    // Setup up publish channel to publish manager
    publishManagerPublishSocket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(publishManagerPublishSocket, PUB_MGR_PUB_URL);

    // Setup the publish manager
    pthread_create(&publishManagerThread, NULL, startPublishManager, context);

    // Setup up communication channel to request manager
    requestManagerSocket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(requestManagerSocket, "inproc://gravity_request_manager");

    // Setup the request manager
    pthread_create(&requestManagerThread, NULL, startRequestManager, context);

    // Setup the service manager
    pthread_create(&serviceManagerThread, NULL, startServiceManager, context);

    // Start the metrics manager
    pthread_create(&metricsManagerThread, NULL, startMetricsManager, context);

    // Configure to trap Ctrl-C (SIGINT) and SIGTERM signals
    s_catch_signals();

    // wait for the manager threads to signal their readiness
    string msgText;
    int numThreadsWaiting = 5;
    while (numThreadsWaiting && !s_interrupted)
    {
    	// Read message
    	msgText = readStringMessage(initSocket);

    	// respond with ack
    	sendStringMessage(initSocket, "ack", ZMQ_DONTWAIT);

    	// Decrement counter
    	numThreadsWaiting--;
    }
    zmq_close(initSocket);

	s_restore_signals();

	if(s_interrupted)
		raise(s_interrupted);

	// connect down here to make sure manager has bound address.
	publishManagerRequestSocket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(publishManagerRequestSocket, PUB_MGR_REQ_URL);

    serviceManagerSocket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(serviceManagerSocket, SERVICE_MGR_URL);

	////////////////////////////////////////////////////////
	//Now that Gravity is set up, get gravity configuration.
	parser = new GravityConfigParser(componentID);

	parser->ParseConfigFile("Gravity.ini");
	std::string config_file_name = componentID + ".ini";
	if(gravity::IsValidFilename(config_file_name))
	{
		parser->ParseConfigFile(config_file_name.c_str());
	}

	//Set Service Directory URL (because we can't connect to the ConfigService without it).
    std::string serviceDirectoryUrl = parser->getString("ServiceDirectoryURL");
    size_t pos = serviceDirectoryUrl.find_first_of("://");
    if(pos != std::string::npos)
    {
    	serviceDirectoryNode.transport = serviceDirectoryUrl.substr(0, pos);
    	pos += 3;
    }
    else
    {
    	serviceDirectoryNode.transport = "tcp";
    	pos = 0;
    }

    size_t pos1 = serviceDirectoryUrl.find_first_of(":", pos);
    serviceDirectoryNode.ipAddress = serviceDirectoryUrl.substr(pos, pos1 - pos);

    if(serviceDirectoryNode.ipAddress == "")
    	serviceDirectoryNode.ipAddress = "localhost";
   	serviceDirectoryNode.port = gravity::StringToInt(serviceDirectoryUrl.substr(pos1 + 1), 5555);

   	if(componentID != "ConfigServer" && getBoolParam("NoConfigServer", false) != true)
   	{
   		parser->ParseConfigService(*this); //Although this is done last, this has the least priority.  We just need to do it last so we know where the service directory is located.
   	}
	//parser->ParseCmdLine

	//Setup Logging if enabled.
   	Log::LogLevel local_log_level = Log::LogStringToLevel(getStringParam("LocalLogLevel", "warning").c_str());
    if(local_log_level != Log::NONE)
        Log::initAndAddFileLogger(getStringParam("LogDirectory", "").c_str(), componentID.c_str(),
                local_log_level, getBoolParam("CloseLogFileAfterWrite", false));

    Log::LogLevel console_log_level = Log::LogStringToLevel(getStringParam("ConsoleLogLevel", "warning").c_str());
    if(console_log_level != Log::NONE)
        Log::initAndAddConsoleLogger(componentID.c_str(), console_log_level);

    Log::LogLevel net_log_level = Log::LogStringToLevel(getStringParam("NetLogLevel", "none").c_str());
	if(net_log_level != Log::NONE)
		Log::initAndAddGravityLogger(this, net_log_level);

    // Enable metrics (if configured)
    metricsEnabled = getBoolParam("GravityMetricsEnabled", false);
    if (metricsEnabled)
    {
        // Register our metrics data product with the service directory
        registerDataProduct(GRAVITY_METRICS_DATA_PRODUCT_ID, GravityTransportTypes::TCP);

        // Command the GravityMetricsManager thread to start collecting metrics
        sendStringMessage(metricsManagerSocket, "MetricsEnable", ZMQ_SNDMORE);

        // Get collection parameters from ini file
        // (default to 10 second sampling, publishing once per min)
        int samplePeriod = getIntParam("GravityMetricsSamplePeriodSeconds", 10);
        int samplesPerPublish = getIntParam("GravityMetricsSamplesPerPublish", 6);

        // Send collection details to the GravityMetricsManager
        sendIntMessage(metricsManagerSocket, samplePeriod, ZMQ_SNDMORE);
        sendIntMessage(metricsManagerSocket, samplesPerPublish, ZMQ_SNDMORE);

        // Finally, send our component id & ip address (to be published with metrics)
        sendStringMessage(metricsManagerSocket, componentID, ZMQ_SNDMORE);
        sendStringMessage(metricsManagerSocket, getIP(), ZMQ_DONTWAIT);
    }

	return ret;
}

void GravityNode::waitForExit()
{
	// Wait on the subscription manager thread
	pthread_join(subscriptionManagerThread, NULL);
}

GravityReturnCode GravityNode::sendRequestsToServiceProvider(string url, const GravityDataProduct& request,
        GravityDataProduct& response, int timeout_in_milliseconds, int retries)
{
    GravityReturnCode ret = GravityReturnCodes::FAILURE;

    int retriesLeft = retries;
    while(retriesLeft && ret != GravityReturnCodes::INTERRUPTED && ret != GravityReturnCodes::SUCCESS)
    {
    	--retriesLeft;
    	ret = sendRequestToServiceProvider(url, request, response, timeout_in_milliseconds);
    }

    return ret;
}

GravityReturnCode GravityNode::sendRequestToServiceProvider(string url, const GravityDataProduct& request,
        GravityDataProduct& response, int timeout_in_milliseconds)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

	// Connect to service provider
    void* socket = zmq_socket(context, ZMQ_REQ); // Socket to connect to service provider
	zmq_connect(socket, url.c_str());

	// Send message to service provider
	sendGravityDataProduct(socket, request, ZMQ_DONTWAIT);

	// Poll socket for reply with a timeout
	zmq_pollitem_t items[] = {{socket, 0, ZMQ_POLLIN, 0}};
	int rc = zmq_poll(items, 1, timeout_in_milliseconds);
	if (rc == -1)
		ret = GravityReturnCodes::INTERRUPTED;
	else if(rc == 0)
		ret = GravityReturnCodes::REQUEST_TIMEOUT;
	// Got a Response, now process it.  Process the response
	else if(items[0].revents & ZMQ_POLLIN)
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
		catch (char*)
		{
			parserSuccess = false;
		}

		// Clean up message
		zmq_msg_close(&resp);

		if (parserSuccess)
			ret = GravityReturnCodes::SUCCESS;
		else
			// Bad response.
			ret = GravityReturnCodes::LINK_ERROR;
	}
	else
		ret = GravityReturnCodes::NO_SERVICE_PROVIDER;

	// Close socket
	zmq_close(socket);

	if(s_interrupted)
	{
		ret = GravityReturnCodes::INTERRUPTED;
		raise(s_interrupted);
	}

    return ret;
}


GravityReturnCode GravityNode::sendRequestToServiceDirectory(const GravityDataProduct& request,
        GravityDataProduct& response)
{
	stringstream ss;
	ss << serviceDirectoryNode.transport << "://" << serviceDirectoryNode.ipAddress <<
	                ":" << serviceDirectoryNode.port;
	string serviceDirectoryURL = ss.str();

	return sendRequestsToServiceProvider(serviceDirectoryURL, request, response, NETWORK_TIMEOUT, NETWORK_RETRIES);
}

GravityReturnCode GravityNode::registerDataProduct(string dataProductID, GravityTransportType transportType)
{
    static Semaphore lock;
    std::string transportType_str;
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    if (publishMap.count(dataProductID) > 0)
    {
        Log::warning("attempt to register duplicate data product ID: %s", dataProductID.c_str());
        return GravityReturnCodes::SUCCESS;
    }

    string endpoint;
    if(transportType == GravityTransportTypes::TCP)
    {
        transportType_str = "tcp";
    	  endpoint = getIP();
    }
#ifndef WIN32
    else if (transportType == GravityTransportTypes::IPC)
    {
        transportType_str = "ipc";
        endpoint = "/tmp/" + dataProductID;
    }
#endif
    else if(transportType == GravityTransportTypes::INPROC)
    {
        transportType_str = "inproc";
    	  endpoint = dataProductID;
    }
    else if(transportType == GravityTransportTypes::PGM)
    {
        transportType_str = "pgm";
    	  endpoint = dataProductID;
    }
    else if(transportType == GravityTransportTypes::EPGM)
    {
        transportType_str = "epgm";
    	  endpoint = dataProductID;
    }

    // we can't allow multiple threads to make request calls to the pub manager at the same time
    // because the requests will step on each other.
    lock.Lock();

    // Send publish details via the request socket.  This allows us to retrieve
    // register url in response so that we can register with the ServiceDirectory.
	sendStringMessage(publishManagerRequestSocket, "register", ZMQ_SNDMORE);
	sendStringMessage(publishManagerRequestSocket, dataProductID, ZMQ_SNDMORE);
    sendStringMessage(publishManagerRequestSocket, transportType_str, ZMQ_SNDMORE);
    if(transportType == GravityTransportTypes::TCP)
    {
        int minPort = getIntParam("MinPort", MIN_PORT);
        int maxPort = getIntParam("MaxPort", MAX_PORT);
        sendIntMessage(publishManagerRequestSocket, minPort, ZMQ_SNDMORE);
        sendIntMessage(publishManagerRequestSocket, maxPort, ZMQ_SNDMORE);
    }
    sendStringMessage(publishManagerRequestSocket, endpoint, ZMQ_DONTWAIT);

	string connectionURL = readStringMessage(publishManagerRequestSocket);

    lock.Unlock();

	if (connectionURL.size() == 0)
	{
	    ret = GravityReturnCodes::NO_PORTS_AVAILABLE;
	}
	else
	{
        if (!serviceDirectoryNode.ipAddress.empty())
        {
            // Create the object describing the data product to register
            ServiceDirectoryRegistrationPB registration;
            registration.set_id(dataProductID);
            registration.set_url(connectionURL);
            registration.set_type(ServiceDirectoryRegistrationPB::DATA);
            registration.set_component_id(componentID);

            // Wrap request in GravityDataProduct
            GravityDataProduct request("RegistrationRequest");
            request.setData(registration);

            // GravityDataProduct for response
            GravityDataProduct response("RegistrationResponse");

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
                catch (char*)
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
	}

	if (ret != GravityReturnCodes::SUCCESS)
	{
	    Log::warning("Failed to register %s at url %s with error %s", dataProductID.c_str(), connectionURL.c_str(), getCodeString(ret).c_str());
	    // if we didn't succesfully register with the SD, then unregister with the publish manager
        sendStringMessage(publishManagerPublishSocket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(publishManagerPublishSocket, dataProductID, ZMQ_DONTWAIT);
	}
	else
	{
	    Log::debug("Registered publisher at address: %s", connectionURL.c_str());
        publishMap[dataProductID] = connectionURL;
	}

	return ret;
}

GravityReturnCode GravityNode::unregisterDataProduct(string dataProductID)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    if (publishMap.count(dataProductID) == 0)
    {
        ret = GravityReturnCodes::REGISTRATION_CONFLICT;
    }
    else
    {
    	sendStringMessage(publishManagerPublishSocket, "unregister", ZMQ_SNDMORE);
    	sendStringMessage(publishManagerPublishSocket, dataProductID, ZMQ_DONTWAIT);
    	string url = publishMap[dataProductID];
        publishMap.erase(dataProductID);

        if (!serviceDirectoryNode.ipAddress.empty())
        {
            ServiceDirectoryUnregistrationPB unregistration;
            unregistration.set_id(dataProductID);
            unregistration.set_url(url);
            unregistration.set_type(ServiceDirectoryUnregistrationPB::DATA);

            GravityDataProduct request("UnregistrationRequest");
            request.setData(unregistration);

            // GravityDataProduct for response
            GravityDataProduct response("UnregistrationResponse");

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
                catch (char*)
                {
                    parserSuccess = false;
                }

                if (parserSuccess)
                {
                    switch (pb.returncode())
                    {
                    case ServiceDirectoryResponsePB::SUCCESS:
                    case ServiceDirectoryResponsePB::NOT_REGISTERED:
                        ret = GravityReturnCodes::SUCCESS;
                        break;
                    default:
                        ret = GravityReturnCodes::FAILURE;
                        break;
                    }
                }
                else
                {
                    ret = GravityReturnCodes::LINK_ERROR;
                }
            }
        }
    }

    return ret;
}

GravityReturnCode GravityNode::ServiceDirectoryDataProductLookup(std::string dataProductID, vector<std::string> &urls)
{
    // Create the object describing the data product to lookup
    ComponentLookupRequestPB lookup;
    lookup.set_lookupid(dataProductID);
    lookup.set_type(ComponentLookupRequestPB::DATA);

    // Wrap request in GravityDataProduct
    GravityDataProduct request("ComponentLookupRequest");
    request.setData(lookup);

    // GravityDataProduct for response
    GravityDataProduct response("ComponentLookupResponse");

    // Send request to service directory
    GravityReturnCode ret = sendRequestToServiceDirectory(request, response);

    if (ret == GravityReturnCodes::SUCCESS)
    {
        ComponentDataLookupResponsePB pb;
        bool parserSuccess = true;
        try
        {
            response.populateMessage(pb);
        }
        catch (char*)
        {
            parserSuccess = false;
        }

        if (parserSuccess)
        {
            for (int i = 0; i < pb.url_size(); i++)
                urls.push_back(pb.url(i));
            ret = GravityReturnCodes::SUCCESS;
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

GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter)
{
	vector<string> url;

	GravityReturnCode ret;
	ret = ServiceDirectoryDataProductLookup(dataProductID, url);
	if(ret != GravityReturnCodes::SUCCESS)
		return ret;

	if (url.size() == 0)
	{
	    subscribe("", dataProductID, subscriber, filter);
	}
	else
	{
        // Subscribe to all published data products
        for (size_t i = 0; i < url.size(); i++)
        {
            subscribe(url[i], dataProductID, subscriber, filter);
        }
	}

	return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::subscribe(string connectionURL, string dataProductID,
        const GravitySubscriber& subscriber, string filter)
{
	vector<string> url;
	GravityReturnCode ret;
	int tries = 5;
	while (url.size() == 0 && tries-- > 0)
	{
		ret = ServiceDirectoryDataProductLookup("RegisteredPublishers", url);
		if(ret != GravityReturnCodes::SUCCESS)
			return ret;
		if (url.size() > 1)
			Log::warning("Found more than one (%d) Service Directory registered for publisher updates?", url.size());
		else if (url.size() == 1)
			break;

		if (tries > 0)
			gravity::sleep(500);
	}

	if (url.size() == 0)
	{
		Log::critical("Service Directory has not finished initialization (RegisteredPublishers not available)");
		return GravityReturnCodes::NO_SERVICE_DIRECTORY;
	}

	// Send subscription details
	sendStringMessage(subscriptionManagerSocket, "subscribe", ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSocket, dataProductID, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSocket, connectionURL, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSocket, filter, ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSocket, url[0], ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(&subscriber));
	void* v = (void*)&subscriber;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&subscriber));
	zmq_sendmsg(subscriptionManagerSocket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::unsubscribe(string dataProductID, const GravitySubscriber& subscriber, string filter)
{
	// Send unsubscribe details
	sendStringMessage(subscriptionManagerSocket, "unsubscribe", ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSocket, dataProductID, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSocket, filter, ZMQ_SNDMORE);

	// Send subscriber
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(&subscriber));
	void* v = (void*)&subscriber;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&subscriber));
	zmq_sendmsg(subscriptionManagerSocket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::publish(const GravityDataProduct& dataProduct, std::string filterText)
{
    string dataProductID = dataProduct.getDataProductID();

    //Set Timestamp
    dataProduct.setTimestamp(getCurrentTime());

	// Send subscription details
	sendStringMessage(publishManagerPublishSocket, "publish", ZMQ_SNDMORE);
	sendStringMessage(publishManagerPublishSocket, filterText, ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&msg));
	zmq_sendmsg(publishManagerPublishSocket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::ServiceDirectoryServiceLookup(std::string serviceID, std::string &url)
{
	// Create the object describing the data product to lookup
	ComponentLookupRequestPB lookup;
	lookup.set_lookupid(serviceID);
	lookup.set_type(ComponentLookupRequestPB::SERVICE);

	// Wrap request in GravityDataProduct
	GravityDataProduct requestDataProduct("ComponentLookupRequest");
	requestDataProduct.setData(lookup);

	// GravityDataProduct for response
	GravityDataProduct responseDataProduct("ComponentLookupResponse");

	// Send request to service directory
	GravityReturnCode ret = sendRequestToServiceDirectory(requestDataProduct, responseDataProduct);

	if (ret == GravityReturnCodes::SUCCESS)
	{
		ComponentServiceLookupResponsePB pb;
		bool parserSuccess = true;
		try
		{
			responseDataProduct.populateMessage(pb);
		}
		catch (char*)
		{
			parserSuccess = false;
		}

		if (parserSuccess)
		{

			if (!pb.url().empty())
			{
				url = pb.url();
				return GravityReturnCodes::SUCCESS;
			}
			else
			{
				ret = GravityReturnCodes::NO_SUCH_SERVICE;
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

//Asynchronous Request with Service Directory Lookup
GravityReturnCode GravityNode::request(string serviceID, const GravityDataProduct& dataProduct,
        const GravityRequestor& requestor, string requestID, int timeout_milliseconds)
{
	std::string url;
	GravityReturnCode ret = ServiceDirectoryServiceLookup(serviceID, url);
	if(ret != GravityReturnCodes::SUCCESS)
		return ret;
	return request(url, serviceID, dataProduct, requestor, requestID, timeout_milliseconds);
}

//Asynchronous Request with URL
GravityReturnCode GravityNode::request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
        const GravityRequestor& requestor, string requestID, int timeout_milliseconds)
{
	// Send subscription details
	sendStringMessage(requestManagerSocket, "request", ZMQ_SNDMORE);
	sendStringMessage(requestManagerSocket, serviceID, ZMQ_SNDMORE);
	sendStringMessage(requestManagerSocket, connectionURL, ZMQ_SNDMORE);
	sendStringMessage(requestManagerSocket, requestID, ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&msg));
	zmq_sendmsg(requestManagerSocket, &msg, ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	zmq_msg_init_size(&msg, sizeof(&requestor));
	void* v = (void*)&requestor;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&requestor));
	zmq_sendmsg(requestManagerSocket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

	return GravityReturnCodes::SUCCESS;
}

//Synchronous Request
shared_ptr<GravityDataProduct> GravityNode::request(string serviceID, const GravityDataProduct& request, int timeout_milliseconds)
{
	std::string connectionURL;
	GravityReturnCode ret = ServiceDirectoryServiceLookup(serviceID, connectionURL);
	if(ret != GravityReturnCodes::SUCCESS)
	{
		Log::warning("Unable to find service %s: %s", serviceID.c_str(), getCodeString(ret).c_str());
		return shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
	}

	shared_ptr<GravityDataProduct> response(new GravityDataProduct(serviceID));
	ret = sendRequestToServiceProvider(connectionURL, request, *response, timeout_milliseconds);
	if(ret != GravityReturnCodes::SUCCESS)
	{
		Log::warning("service request returned error: %s", getCodeString(ret).c_str());
		return shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
	}

	return response;
}

GravityReturnCode GravityNode::registerService(string serviceID, GravityTransportType transportType,
        const GravityServiceProvider& server)
{
    string transportType_str;
    if (serviceMap.count(serviceID) > 0)
    {
        Log::warning("attempt to register duplicate service ID: %s", serviceID.c_str());
        return GravityReturnCodes::SUCCESS;
    }


    // Build the connection string
    string endpoint;
    if(transportType == GravityTransportTypes::TCP)
    {
        transportType_str = "tcp";
        endpoint = getIP();
    }
#ifndef WIN32
    else if (transportType == GravityTransportTypes::IPC)
    {
        transportType_str = "ipc";
        endpoint = "/tmp/" + serviceID;
    }
#endif
    else if(transportType == GravityTransportTypes::INPROC)
    {
        transportType_str ="inproc";
    	  endpoint = serviceID;
    }
    else if(transportType == GravityTransportTypes::PGM)
    {
        transportType_str ="pgm";
    	  endpoint = serviceID;
    }
    else if(transportType == GravityTransportTypes::EPGM)
    {
        transportType_str ="epgm";
    	  endpoint = serviceID;
    }

	// Send subscription details
	sendStringMessage(serviceManagerSocket, "register", ZMQ_SNDMORE);
	sendStringMessage(serviceManagerSocket, serviceID, ZMQ_SNDMORE);
	sendStringMessage(serviceManagerSocket, transportType_str, ZMQ_SNDMORE);

    if(transportType == GravityTransportTypes::TCP)
    {
        int minPort = getIntParam("MinPort", MIN_PORT);
        int maxPort = getIntParam("MaxPort", MAX_PORT);
        sendIntMessage(serviceManagerSocket, minPort, ZMQ_SNDMORE);
        sendIntMessage(serviceManagerSocket, maxPort, ZMQ_SNDMORE);
    }
    sendStringMessage(serviceManagerSocket, endpoint, ZMQ_SNDMORE);


	// Include the server
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(&server));
	void* v = (void*)&server;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&server));
	zmq_sendmsg(serviceManagerSocket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    string connectionURL = readStringMessage(serviceManagerSocket);

    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    if (ret == GravityReturnCodes::SUCCESS && !serviceDirectoryNode.ipAddress.empty())
    {
        // Create the object describing the data product to register
        ServiceDirectoryRegistrationPB registration;
        registration.set_id(serviceID);
        registration.set_url(connectionURL);
        registration.set_type(ServiceDirectoryRegistrationPB::SERVICE);
        registration.set_component_id(componentID);

        // Wrap request in GravityDataProduct
        GravityDataProduct request("RegistrationRequest");
        request.setData(registration);

        // GravityDataProduct for response
        GravityDataProduct response("RegistrationResponse");

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
            catch (char*)
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

    if (ret != GravityReturnCodes::SUCCESS)
    {
        // unregister the service from the service manager if we couldn't register with the SD
        sendStringMessage(serviceManagerSocket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(serviceManagerSocket, serviceID, ZMQ_DONTWAIT);
        // wait for it to return
        readStringMessage(serviceManagerSocket);
    }
    else
    {
        Log::debug("Registered service at address: %s", connectionURL.c_str());
        serviceMap[serviceID] = connectionURL;
    }
    return ret;
}

GravityReturnCode GravityNode::unregisterService(string serviceID)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;
    if (serviceMap.count(serviceID) == 0)
    {
        ret = GravityReturnCodes::REGISTRATION_CONFLICT;
    }
    else
    {
        sendStringMessage(serviceManagerSocket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(serviceManagerSocket, serviceID, ZMQ_DONTWAIT);
        string url = serviceMap[serviceID];
        serviceMap.erase(serviceID);

        string status = readStringMessage(serviceManagerSocket);

        if (!serviceDirectoryNode.ipAddress.empty())
        {
            ServiceDirectoryUnregistrationPB unregistration;
            unregistration.set_id(serviceID);
            unregistration.set_url(url);
            unregistration.set_type(ServiceDirectoryUnregistrationPB::SERVICE);

            GravityDataProduct request("UnregistrationRequest");
            request.setData(unregistration);

            // GravityDataProduct for response
            GravityDataProduct response("UnregistrationResponse");

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
                catch (char*)
                {
                    parserSuccess = false;
                }

                if (parserSuccess)
                {
                    switch (pb.returncode())
                    {
                    case ServiceDirectoryResponsePB::SUCCESS:
                    case ServiceDirectoryResponsePB::NOT_REGISTERED:
                        ret = GravityReturnCodes::SUCCESS;
                        break;
                    default:
                        ret = GravityReturnCodes::FAILURE;
                        break;
                    }
                }
                else
                {
                    ret = GravityReturnCodes::LINK_ERROR;
                }
            }
        }
    }

    return ret;
}

GravityReturnCode GravityNode::startHeartbeat(int64_t interval_in_microseconds)
{
	if(interval_in_microseconds < 0)
		return gravity::GravityReturnCodes::FAILURE;

	static bool started = false;

	if(started)
		return gravity::GravityReturnCodes::FAILURE; //We shouldn't be able to start this guy twice

	this->registerDataProduct(componentID, GravityTransportTypes::TCP);

	HBParams* params = new HBParams(); //(freed by thread)
	params->zmq_context = context;
	params->interval_in_microseconds = interval_in_microseconds;
	params->componentID = componentID;
	params->minPort = getIntParam("MinPort", MIN_PORT);
	params->maxPort = getIntParam("MaxPort", MAX_PORT);
	params->endpoint = getIP();

	pthread_t heartbeatThread;
	pthread_create(&heartbeatThread, NULL, Heartbeat, (void*)params);

	return gravity::GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::registerHeartbeatListener(string componentID, int64_t timebetweenMessages, const GravityHeartbeatListener& listener)
{
	void* HeartbeatListener(void*); //Forward declaration.
	static class Heartbeat hbSub;

	if(hbSocket == NULL)
	{
		//Initialize Heartbeat thread.
		hbSocket = zmq_socket(context, ZMQ_REQ);
		zmq_bind(hbSocket, "inproc://heartbeat_listener");
		HBListenerContext* thread_context = new HBListenerContext();
		thread_context->zmq_context = this->context;
		pthread_t heartbeatListenerThread;
		pthread_create(&heartbeatListenerThread, NULL, Heartbeat::HeartbeatListenerThrFunc, thread_context);
	}

	this->subscribe(componentID, hbSub);

	//Send the DataproductID
	sendStringMessage(hbSocket, componentID, ZMQ_SNDMORE);

	//Send the address of the listener
	zmq_msg_t msg1;
	zmq_msg_init_size(&msg1, sizeof(GravityHeartbeatListener*));
	intptr_t p = (intptr_t) &listener;
	memcpy(zmq_msg_data(&msg1), &p, sizeof(GravityHeartbeatListener*));
	zmq_sendmsg(hbSocket, &msg1, ZMQ_SNDMORE);
	zmq_msg_close(&msg1);

	//Send the Max time between messages
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, 8);
	memcpy(zmq_msg_data(&msg), &timebetweenMessages, 8);
	zmq_sendmsg(hbSocket, &msg, 0);
	zmq_msg_close(&msg);

	// Read the ACK
	readStringMessage(hbSocket);

	return GravityReturnCodes::SUCCESS;
}

#ifdef WIN32
//convert binary address to string.
const char *inet_ntop(int af, const void * src, char* dest, int dest_length)
{
	assert(af == AF_INET); //We only support IPV4

	unsigned short new_src[4];
	new_src[0] = (uint8_t) ((char *) src)[0];
	new_src[1] = (uint8_t) ((char *) src)[1];
	new_src[2] = (uint8_t) ((char *) src)[2];
	new_src[3] = (uint8_t) ((char *) src)[3];
	std::stringstream ss;
	ss << new_src[0] << "." << new_src[1]  << "." << new_src[2]   << "." << new_src[3]; //TODO: verify Byte Order.
	if(dest_length < (int) ss.str().length() + 1)
		return NULL;

	memcpy(dest, ss.str().c_str(), ss.str().length() + 1);

	return dest;
}

typedef int socklen_t;
#endif

string GravityNode::getIP()
{
    string ip = "127.0.0.1";

    if (!serviceDirectoryNode.ipAddress.empty() && serviceDirectoryNode.ipAddress != "localhost")
    {
		//Reads the IP used to connect to the Service Directory.
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

#ifdef WIN32
		closesocket(sock);
#else
        close(sock);
#endif

        ip.assign(buffer);
    }

    return ip;
}

std::string GravityNode::getStringParam(std::string key, std::string default_value)
{
	return parser->getString(key, default_value);
}

int GravityNode::getIntParam(std::string key, int default_value)
{
	std::string value = parser->getString(key, "");

	return StringToInt(value, default_value);
}

double GravityNode::getFloatParam(std::string key, double default_value)
{
	std::string value = parser->getString(key, "");

	return StringToDouble(value, default_value);
}

bool GravityNode::getBoolParam(std::string key, bool default_value)
{
    string val = StringToLowerCase(parser->getString(key, default_value ? "true" : "false"));
	if( val == "true" ||
		val == "t" ||
		val == "yes" ||
		val == "y" )
		return true;
	else
		return false;
}

static std::map<GravityReturnCode,std::string> code_strings =
  boost::assign::map_list_of
    (GravityReturnCodes::SUCCESS, "SUCCESS")
    (GravityReturnCodes::FAILURE, "FAILURE")
    (GravityReturnCodes::NO_SERVICE_DIRECTORY, "NO_SERVICE_DIRECTORY")
    (GravityReturnCodes::REQUEST_TIMEOUT, "REQUEST_TIMEOUT")
    (GravityReturnCodes::DUPLICATE, "DUPLICATE")
    (GravityReturnCodes::REGISTRATION_CONFLICT, "REGISTRATION_CONFLICT")
    (GravityReturnCodes::NOT_REGISTERED, "NOT_REGISTERED")
    (GravityReturnCodes::NO_SUCH_SERVICE, "NO_SUCH_SERVICE")
    (GravityReturnCodes::LINK_ERROR, "LINK_ERROR")
    (GravityReturnCodes::INTERRUPTED, "INTERRUPTED")
    (GravityReturnCodes::NO_SERVICE_PROVIDER, "NO_SERVICE_PROVIDER")
    (GravityReturnCodes::NO_PORTS_AVAILABLE, "NO_PORTS_AVAILABLE");

string GravityNode::getCodeString(GravityReturnCode code) {
    std::string s;
    if (code_strings.count(code) == 0) {
        ostringstream convert;
        convert << code;
        s = convert.str();
    } else {
        s = code_strings[code];
    }
    return s;
}

} /* namespace gravity */
