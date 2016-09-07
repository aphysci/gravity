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
#include <WinBase.h>
#include <Windows.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
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
#include "FutureResponse.h"

#include "protobuf/ComponentLookupRequestPB.pb.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ComponentServiceLookupResponsePB.pb.h"
#include "protobuf/ServiceDirectoryResponsePB.pb.h"
#include "protobuf/ServiceDirectoryRegistrationPB.pb.h"
#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"
#include "protobuf/ServiceDirectoryBroadcastPB.pb.h"

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

void* GravityNode::startGravityDomainListener(void* context)
{
	//Create and start  the GravityNodeDomainListener
	GravityNodeDomainListener domainListener(context);
	domainListener.start();

	return NULL;
}

//Forward Declarations that we don't want publicly visible (Need to be in gravity namespace).
bool IsValidFilename(const std::string filename);
int StringToInt(std::string str, int default_value);
double StringToDouble(std::string str, double default_value);
void* Heartbeat(void* thread_context);


using namespace std;
using namespace std::tr1;

GravityNode::GravityNodeDomainListener::GravityNodeDomainListener(void* context)
{
	this->context=context;
	sock = 0;
	running = false;
}

GravityNode::GravityNodeDomainListener::~GravityNodeDomainListener()
{
	#ifdef _WIN32
	closesocket(sock);
	#else
	close(sock);
	#endif
}

void GravityNode::GravityNodeDomainListener::start()
{
	ready=false;

	gravityNodeSocket = zmq_socket(context,ZMQ_REP);
	zmq_connect(gravityNodeSocket,"inproc://gravity_domain_listener");
	zmq_setsockopt(gravityNodeSocket, ZMQ_SUBSCRIBE, NULL, 0);

	void* domainSocket=zmq_socket(context,ZMQ_PUB);
	zmq_connect(domainSocket,"inproc://gravity_domain_receiver");

	// Poll the gravity node
	zmq_pollitem_t pollItem;
	pollItem.socket = gravityNodeSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;


	while(!ready)
	{
		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&pollItem, 1, -1); // 0 --> return immediately, -1 --> blocks
		if (rc == -1 )
		{
		    Log::debug("Interrupted, exiting (rc = %d)", rc);
			// Interrupted
			break;
		}

		// Process new subscription requests from the gravity node
		if (pollItem.revents & ZMQ_POLLIN)
		{
			std::string command = readStringMessage(gravityNodeSocket);
			if(command=="configure")
			{
				readDomainListenerParameters();
				ready=true;
			}
		}
	}

	time_t serviceDirectoryStartTime = 0;

	/* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
		return;
    }


    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(port);      /* Broadcast port */

	//set socket to be re-usable. Must be set for all other listeners for this port
	int one = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const char*)&one,sizeof(one));

    /* Bind to the broadcast port */
	int rc = 0;
    if ((rc = bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr))) < 0)
    {
		return;
    }

	char recvString[MAXRECVSTRING+1]; /* Buffer for received string */
    int recvStringLen;                /* Length of received string */
	ServiceDirectoryBroadcastPB broadcastPB;

	struct timeval timetowait;
	timetowait.tv_sec=timeout;
	timetowait.tv_usec=0;

	//set socket to block forever
#ifdef _WIN32
	int timeout_int = timevalToMilliSeconds(&timetowait);
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout_int,sizeof(unsigned int));
#else
	//set socket to block forever initially
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timetowait,sizeof(struct timeval));
#endif


	running = true;

	while(running)
	{
		//wait for braodcast message to be recieved
		memset(recvString,0,MAXRECVSTRING+1);

		/* Receive a broadcast message or timeout */
		recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0);
		
		//check for socket error
		if(recvStringLen < 0)
		{
			//if we received some error
		    int error = errno;
		    if (error == EAGAIN)
		        Log::trace("Timed out waiting for domain broadcast");
		    else
		        Log::warning("Received error reading domain listener socket: %d", error);

		}
		else //we received a message
		{
			broadcastPB.ParseFromArray(recvString,recvStringLen);
			Log::trace("Domain listener received message from domain '%s'", broadcastPB.domain().c_str());

			//if the domains match
			if(domain.compare(broadcastPB.domain())==0)
			{
                sendStringMessage(domainSocket,"connect",ZMQ_SNDMORE);
				sendStringMessage(domainSocket,domain,ZMQ_SNDMORE);
				sendStringMessage(domainSocket,broadcastPB.url(),ZMQ_DONTWAIT);

				// if it's a new start time...
				if (serviceDirectoryStartTime < broadcastPB.starttime())
				{
				    Log::debug("Domain listener found update to our domain, orig SD start time = %u, new SD start time = %llu, SD url is now %s",
				                serviceDirectoryStartTime, broadcastPB.starttime(), broadcastPB.url().c_str());
				             // If we've seen a start time before, then re-register
				    if (serviceDirectoryStartTime != 0)
				    {
				        gravityNode->ServiceDirectoryReregister(compId);
				    }
				    serviceDirectoryStartTime = broadcastPB.starttime();
				}
			}
		}

		//check for any gravitynode commands
		rc = zmq_poll(&pollItem, 1, 0);
		if (pollItem.revents & ZMQ_POLLIN)
		{
			std::string command = readStringMessage(gravityNodeSocket);
			sendStringMessage(gravityNodeSocket,"ACK",ZMQ_DONTWAIT);
			if(command=="kill")
			{
				running=false;
				break;
			}

		}

		//don't set a timeout on the socket if it has already been closed
		if(running)
		{
			//set new timeout
			#ifdef _WIN32
			setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout_int,sizeof(unsigned int));
			#else
			//set socket to block until the timeout
			setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timetowait,sizeof(struct timeval));
			#endif	
		}

	}//end while

	Log::warning("Closing Domain Receiver");
	#ifdef _WIN32
	closesocket(sock);
	#else
	close(sock);
	#endif
	zmq_close(gravityNodeSocket);
	zmq_close(domainSocket);
}

void GravityNode::GravityNodeDomainListener::readDomainListenerParameters()
{
	//recieve domain
	domain=readStringMessage(gravityNodeSocket);

	//receive port
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(gravityNodeSocket,&msg,ZMQ_DONTWAIT);
	memcpy(&port,zmq_msg_data(&msg),sizeof(unsigned int));
	zmq_msg_close(&msg);

	//receive timeout
	zmq_msg_t msg2;
	zmq_msg_init(&msg2);
	zmq_recvmsg(gravityNodeSocket,&msg2,ZMQ_DONTWAIT);
	memcpy(&timeout,zmq_msg_data(&msg2),sizeof(unsigned int));
	zmq_msg_close(&msg2);

	//receive component ID
	compId=readStringMessage(gravityNodeSocket);

	//receive GravityNode pointer
	zmq_msg_t msg3;
	zmq_msg_init(&msg3);
	zmq_recvmsg(gravityNodeSocket,&msg3,ZMQ_DONTWAIT);
	memcpy(&gravityNode,zmq_msg_data(&msg3),sizeof(GravityNode*));
	zmq_msg_close(&msg3);

	sendStringMessage(gravityNodeSocket,"ACK",ZMQ_DONTWAIT);
}

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
	initialized=false;
	logInitialized = false;
	listenerEnabled=false;
	heartbeatStarted=false;
}

GravityNode::GravityNode(std::string componentID)
{
    // Eventually to be read from a config/properties file
    serviceDirectoryNode.port = 5555;
    serviceDirectoryNode.transport = "tcp";
    serviceDirectoryNode.socket = NULL;

    //Initialize this guy so we can know whether the heartbeat thread has started.
    hbSocket = NULL;

    // Default to no metrics
    metricsEnabled = false;
	initialized=false;
	logInitialized=false;
	heartbeatStarted=false;

	init(componentID);
}

GravityNode::~GravityNode()
{

	// If metrics are enabled, we need to unregister our metrics data product
    if (metricsEnabled)
    {
        unregisterDataProduct(GRAVITY_METRICS_DATA_PRODUCT_ID);
    }

	//closeHeartbeatSocket();

	//kill the domain listener
	if(listenerEnabled)
	{
		sendStringMessage(domainListenerSWL.socket,"kill",ZMQ_DONTWAIT);
		readStringMessage(domainListenerSWL.socket);
		zmq_close(domainListenerSWL.socket);
		zmq_close(domainRecvSWL.socket);
	}

	// Close the inproc sockets
	sendStringMessage(subscriptionManagerSWL.socket, "kill", ZMQ_DONTWAIT);
	zmq_close(subscriptionManagerSWL.socket);

	sendStringMessage(requestManagerSWL.socket, "kill", ZMQ_DONTWAIT);
	zmq_close(requestManagerSWL.socket);
	zmq_close(requestManagerRepSWL.socket);

    sendStringMessage(publishManagerPublishSWL.socket, "kill", ZMQ_DONTWAIT);
    zmq_close(publishManagerPublishSWL.socket);
    zmq_close(publishManagerRequestSWL.socket);

    sendStringMessage(serviceManagerSWL.socket, "kill", ZMQ_DONTWAIT);
    zmq_close(serviceManagerSWL.socket);
	zmq_close(serviceManagerConfigSWL.socket);

    sendStringMessage(metricsManagerSocket, "kill", ZMQ_DONTWAIT);
    zmq_close(metricsManagerSocket);

	// Clean up the zmq context object
    zmq_term(context);

	delete parser;

}

GravityReturnCode GravityNode::init()
{
	////////////////////////////////////////////////////////
	//get gravity configuration.
	parser = new GravityConfigParser("");

	parser->ParseConfigFile("Gravity.ini");

	std::string id = parser->getString("GravityComponentID","");

	if(id != "")
	{
		return init(id);
	}
	else
	{
		componentID="GravityNode";
		//Setup Logging if enabled.
		if(!logInitialized)
		{
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

			//log an error indicating the componentID was missing
			Log::critical("Field 'GravityComponentID' missing from Gravity.ini, using GravityComponentID='GravityNode'");

			logInitialized=true;
		}
		return init(componentID);
	}

}

GravityReturnCode GravityNode::init(std::string componentID)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;
	this->componentID = componentID;

	std::string serviceDirectoryUrl="";
	bool domainTimeout=false;
	bool iniWarning = false;

    // Setup zmq context
	if(!initialized)
	{
		context = zmq_init(1);
		if (!context)
		{
			ret = GravityReturnCodes::FAILURE;
		}

		void* initSocket = zmq_socket(context, ZMQ_REP);
		zmq_bind(initSocket, "inproc://gravity_init");

		// Setup up communication channel to subscription manager
		subscriptionManagerSWL.socket = zmq_socket(context, ZMQ_PUB);
		zmq_bind(subscriptionManagerSWL.socket, "inproc://gravity_subscription_manager");

		// Setup the metrics control communication channel
		metricsManagerSocket = zmq_socket(context, ZMQ_PUB);
		zmq_bind(metricsManagerSocket, GRAVITY_METRICS_CONTROL);

		// Setup the subscription manager
		pthread_create(&subscriptionManagerThread, NULL, startSubscriptionManager, context);

		// Setup up publish channel to publish manager
		publishManagerPublishSWL.socket = zmq_socket(context, ZMQ_PUB);
		zmq_bind(publishManagerPublishSWL.socket, PUB_MGR_PUB_URL);

		// Setup the publish manager
		pthread_create(&publishManagerThread, NULL, startPublishManager, context);

		// Setup up communication channel to request manager
		requestManagerSWL.socket = zmq_socket(context, ZMQ_PUB);
		zmq_bind(requestManagerSWL.socket, "inproc://gravity_request_manager");
		requestManagerRepSWL.socket = zmq_socket(context, ZMQ_REQ);
		zmq_bind(requestManagerRepSWL.socket, "inproc://gravity_request_rep");
		// Setup the request manager
		pthread_create(&requestManagerThread, NULL, startRequestManager, context);

		serviceManagerConfigSWL.socket = zmq_socket(context,ZMQ_PUB);
		zmq_bind(serviceManagerConfigSWL.socket,"inproc://gravity_service_manager_configure");

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
		publishManagerRequestSWL.socket = zmq_socket(context, ZMQ_REQ);
		zmq_connect(publishManagerRequestSWL.socket, PUB_MGR_REQ_URL);

		serviceManagerSWL.socket = zmq_socket(context, ZMQ_REQ);
		zmq_connect(serviceManagerSWL.socket, SERVICE_MGR_URL);

		////////////////////////////////////////////////////////
		//Now that Gravity is set up, get gravity configuration.
		parser = new GravityConfigParser(componentID);

		parser->ParseConfigFile("Gravity.ini");
		std::string config_file_name = componentID + ".ini";
		if(gravity::IsValidFilename(config_file_name))
		{
			parser->ParseConfigFile(config_file_name.c_str());
		}

		// Setup Logging as soon as config parser is available.
		if(!logInitialized)
		{
			Log::LogLevel local_log_level = Log::LogStringToLevel(getStringParam("LocalLogLevel", "warning").c_str());
			if(local_log_level != Log::NONE)
				Log::initAndAddFileLogger(getStringParam("LogDirectory", "").c_str(), componentID.c_str(),
						local_log_level, getBoolParam("CloseLogFileAfterWrite", false));

			Log::LogLevel console_log_level = Log::LogStringToLevel(getStringParam("ConsoleLogLevel", "warning").c_str());
			if(console_log_level != Log::NONE)
				Log::initAndAddConsoleLogger(componentID.c_str(), console_log_level);

			logInitialized=true;
		}

		// Configure high water marks
		int publishHWM = getIntParam("PublishHWM", 1000);
		if (publishHWM < 0)
		{
			Log::warning("Invalid PublishHWM = %d. Ignoring.", publishHWM);
		}
		else
		{
			// Send HWM (REQ/REP)
			sendStringMessage(publishManagerRequestSWL.socket, "set_hwm", ZMQ_SNDMORE);
			sendIntMessage(publishManagerRequestSWL.socket, publishHWM, ZMQ_DONTWAIT);
			// Read ACK
			readStringMessage(publishManagerRequestSWL.socket);
		}
		int subscribeHWM = getIntParam("SubscribeHWM", 1000);
		if (subscribeHWM < 0)
		{
			Log::warning("Invalid subscribeHWM = %d. Ignoring.", subscribeHWM);
		}
		else
		{
			// Send HWM (PUB/SUB)
			sendStringMessage(subscriptionManagerSWL.socket, "set_hwm", ZMQ_SNDMORE);
			sendIntMessage(subscriptionManagerSWL.socket, subscribeHWM, ZMQ_DONTWAIT);
		}

		//get the Domain name of the Service Directory to connect to
		std::string serviceDirectoryDomain = getStringParam("Domain");

		//Set Service Directory URL (because we can't connect to the ConfigService without it).
		serviceDirectoryUrl = getStringParam("ServiceDirectoryURL");

		if(serviceDirectoryDomain != "" && (componentID != "ServiceDirectory"))
		{
			//if the config file specifies both domain and url
			if( serviceDirectoryUrl != "")
			{
				iniWarning=true;
			}
			//setup and start the GravityNodeDomainListener
			else
			{
				// Setup up communication channel to subscription manager
				domainListenerSWL.socket = zmq_socket(context, ZMQ_REQ);
				zmq_bind(domainListenerSWL.socket, "inproc://gravity_domain_listener");

				domainRecvSWL.socket = zmq_socket(context,ZMQ_SUB);
				zmq_bind(domainRecvSWL.socket,"inproc://gravity_domain_receiver");
				zmq_setsockopt(domainRecvSWL.socket,ZMQ_SUBSCRIBE,NULL,0);

				pthread_create(&domainListenerThread,NULL,startGravityDomainListener,context);			

				configureNodeDomainListener(serviceDirectoryDomain);
				int broadcastTimeout = getIntParam("ServiceDirectoryBroadcastTimeout",DEFAULT_BROADCAST_TIMEOUT_SEC);

				serviceDirectoryUrl.assign(getDomainUrl(broadcastTimeout));
				if(serviceDirectoryUrl == "")
				{
					domainTimeout=true;
				}

				listenerEnabled=true;
				
			}
		}

		initialized = true;
	}
	//we are already initialzed, just try to read the SD domain and url
	else
	{
		//get the Domain name of the Service Directory to connect to
		std::string serviceDirectoryDomain = getStringParam("Domain");
		//Set Service Directory URL (because we can't connect to the ConfigService without it).
		serviceDirectoryUrl = getStringParam("ServiceDirectoryURL");

		if(serviceDirectoryDomain != "" && (componentID != "ServiceDirectory"))
		{
			//if the config file specifies both domain and url
			if( serviceDirectoryUrl != "")
			{
				iniWarning=true;
			}
			else
			{
				int broadcastTimeout = getIntParam("ServiceDirectoryBroadcastTimeout",DEFAULT_BROADCAST_TIMEOUT_SEC);

				//read the domain from the domain listener
				serviceDirectoryUrl.assign(getDomainUrl(broadcastTimeout));
				if(serviceDirectoryUrl == "")
				{
					domainTimeout=true;
				}
				
			}
		}
	}

	//If we are able to proceed with trying to connect to the service directory
	if(!domainTimeout)
	{
	    serviceDirectoryLock.Lock();
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

		/* The "*" is for the case where they did not define a URL in the Gravity.ini file,
			So the ServiceDirectory broadcasted a URL with a "*" in it
		*/
		if(serviceDirectoryNode.ipAddress == "" || serviceDirectoryNode.ipAddress == "*")
    		serviceDirectoryNode.ipAddress = "localhost";
   		serviceDirectoryNode.port = gravity::StringToInt(serviceDirectoryUrl.substr(pos1 + 1), 5555);
   	    serviceDirectoryLock.Unlock();

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

		if(componentID != "ConfigServer" && getBoolParam("NoConfigServer", false) != true)
   		{
   			parser->ParseConfigService(*this); //Although this is done last, this has the least priority.  We just need to do it last so we know where the service directory is located.
   		}
		//parser->ParseCmdLine

   		// Setup up network logging now that SD is available
		Log::LogLevel net_log_level = Log::LogStringToLevel(getStringParam("NetLogLevel", "none").c_str());
		if(net_log_level != Log::NONE)
			Log::initAndAddGravityLogger(this, net_log_level);

		// Get our domain
		if (componentID != "ServiceDirectory")
		{
			GravityDataProduct request("GetDomain");
			GravityDataProduct response("DomainResponse");
			sendRequestToServiceDirectory(request, response);

			char* p = (char*)calloc(response.getDataSize(), sizeof(char));
			response.getData(p, response.getDataSize());
			myDomain.assign(p, response.getDataSize());
            free(p);
		}
		else
		{
			myDomain = getStringParam("Domain", "");
		}

		configureServiceManager();
	}
	else
	{
		ret=GravityReturnCodes::FAILURE;
	}

	if (iniWarning)
	{
			Log::warning("Gravity.ini specifies both Domain and URL. Using URL.");
	}

	return ret;
}

void GravityNode::configureNodeDomainListener(std::string domain)
{
	sendStringMessage(domainListenerSWL.socket,"configure",ZMQ_SNDMORE);
	sendStringMessage(domainListenerSWL.socket,domain,ZMQ_SNDMORE);

	int port = getIntParam("ServiceDirectoryBroadcastPort",DEFAULT_BROADCAST_PORT);
	int broadcastTimeout = getIntParam("ServiceDirectoryBroadcastTimeout",DEFAULT_BROADCAST_TIMEOUT_SEC);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg,sizeof(port));
	memcpy(zmq_msg_data(&msg),&port,sizeof(port));
	zmq_sendmsg(domainListenerSWL.socket,&msg,ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	zmq_msg_t msg2;
	zmq_msg_init_size(&msg2,sizeof(broadcastTimeout));
	memcpy(zmq_msg_data(&msg2),&broadcastTimeout,sizeof(broadcastTimeout));
	zmq_sendmsg(domainListenerSWL.socket,&msg2,ZMQ_SNDMORE);
	zmq_msg_close(&msg2);

	sendStringMessage(domainListenerSWL.socket,componentID,ZMQ_SNDMORE);

	zmq_msg_t msg3;
	zmq_msg_init_size(&msg3,sizeof(GravityNode*));
	GravityNode* tmp = this;
	memcpy(zmq_msg_data(&msg3),&tmp,sizeof(GravityNode*));
	zmq_sendmsg(domainListenerSWL.socket,&msg3,ZMQ_DONTWAIT);
	zmq_msg_close(&msg3);

	readStringMessage(domainListenerSWL.socket);
}

void GravityNode::configureServiceManager()
{

	sendStringMessage(serviceManagerConfigSWL.socket,"configure",ZMQ_SNDMORE);

	//send Domain
	sendStringMessage(serviceManagerConfigSWL.socket,myDomain,ZMQ_SNDMORE);

	//Send component ID
	sendStringMessage(serviceManagerConfigSWL.socket,componentID,ZMQ_DONTWAIT);

}

std::string GravityNode::getDomainUrl(int timeout)
{
	int waitTime=0;

	zmq_pollitem_t pollItem;
	pollItem.socket=domainRecvSWL.socket;
	pollItem.events=ZMQ_POLLIN;
	pollItem.fd=0;
	pollItem.revents=0;
	std::string url="";
	while(waitTime < timeout*1000)
	{
		 // Start polling socket(s), blocking while we wait
        int rc = zmq_poll(&pollItem, 1, 0); // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            break;
        }

        // Process new subscription requests from the gravity node
        if (pollItem.revents & ZMQ_POLLIN)
        {
			std::string command=readStringMessage(domainRecvSWL.socket);
			if(command=="connect")
			{
				std::string domain = readStringMessage(domainRecvSWL.socket);
				url=readStringMessage(domainRecvSWL.socket);
				break;
			}
		}

		sleep(100);
		waitTime+=100;
	}
	return url;

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
	Log::trace("GravityNode::sendRequestToServiceProvider(%s,%s,%s,%d)", url.c_str(), request.getDataProductID().c_str(), 
													response.getDataProductID().c_str(), timeout_in_milliseconds);
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
		// Get response from service
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
	{
		ret = GravityReturnCodes::NO_SERVICE_PROVIDER;
	}

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
    serviceDirectoryLock.Lock();
	ss << serviceDirectoryNode.transport << "://" << serviceDirectoryNode.ipAddress <<
	                ":" << serviceDirectoryNode.port;
	string serviceDirectoryURL = ss.str();
	Log::trace("About to make SD request at URL = %s", serviceDirectoryURL.c_str());

	GravityReturnCode ret = sendRequestsToServiceProvider(serviceDirectoryURL, request, response, NETWORK_TIMEOUT, NETWORK_RETRIES);
    serviceDirectoryLock.Unlock();
    return ret;
}

GravityReturnCode GravityNode::registerDataProduct(string dataProductID, GravityTransportType transportType)
{
    std::string transportType_str;
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    // we can't allow multiple threads to make request calls to the pub manager at the same time
    // because the requests will step on each other.  Manage access to publishMap as well.
    publishManagerRequestSWL.lock.Lock();

    if (publishMap.count(dataProductID) > 0)
    {
        Log::warning("attempt to register duplicate data product ID: %s", dataProductID.c_str());
        publishManagerRequestSWL.lock.Unlock();
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

    // Send publish details via the request socket.  This allows us to retrieve
    // register url in response so that we can register with the ServiceDirectory.
	sendStringMessage(publishManagerRequestSWL.socket, "register", ZMQ_SNDMORE);
	sendStringMessage(publishManagerRequestSWL.socket, dataProductID, ZMQ_SNDMORE);
    sendStringMessage(publishManagerRequestSWL.socket, transportType_str, ZMQ_SNDMORE);
    if(transportType == GravityTransportTypes::TCP)
    {
        int minPort = getIntParam("MinPort", MIN_PORT);
        int maxPort = getIntParam("MaxPort", MAX_PORT);
        sendIntMessage(publishManagerRequestSWL.socket, minPort, ZMQ_SNDMORE);
        sendIntMessage(publishManagerRequestSWL.socket, maxPort, ZMQ_SNDMORE);
    }
    sendStringMessage(publishManagerRequestSWL.socket, endpoint, ZMQ_DONTWAIT);

	string connectionURL = readStringMessage(publishManagerRequestSWL.socket);
	uint64_t timestamp = getCurrentTime();

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
			registration.set_timestamp(timestamp);

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
	    sendStringMessage(publishManagerRequestSWL.socket, "unregister", ZMQ_SNDMORE);
	    sendStringMessage(publishManagerRequestSWL.socket, dataProductID, ZMQ_DONTWAIT);
	    readStringMessage(publishManagerRequestSWL.socket);
	}
	else
	{
	    Log::debug("Registered publisher at address: %s", connectionURL.c_str());
        publishMap[dataProductID] = connectionURL;
		urlInstanceMap[connectionURL] = timestamp;
	}

    publishManagerRequestSWL.lock.Unlock();

	return ret;
}

GravityReturnCode GravityNode::unregisterDataProduct(string dataProductID)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    publishManagerRequestSWL.lock.Lock();
    if (publishMap.count(dataProductID) == 0)
    {
        ret = GravityReturnCodes::REGISTRATION_CONFLICT;
    }
    else
    {
        sendStringMessage(publishManagerRequestSWL.socket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(publishManagerRequestSWL.socket, dataProductID, ZMQ_DONTWAIT);
        readStringMessage(publishManagerRequestSWL.socket);
    	string url = publishMap[dataProductID];
        publishMap.erase(dataProductID);
		urlInstanceMap.erase(url);

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
    publishManagerRequestSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::ServiceDirectoryDataProductLookup(std::string dataProductID, vector<std::string> &urls, string& domain)
{
    // Create the object describing the data product to lookup
    ComponentLookupRequestPB lookup;
    lookup.set_lookupid(dataProductID);
	lookup.set_domain_id(domain);
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

GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter, string domain)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret = subscribeInternal(dataProductID, subscriber, filter, domain);
    subscriptionManagerSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::subscribeInternal(string dataProductID, const GravitySubscriber& subscriber, string filter, string domain)
{
    if (domain.empty())
    {
        domain = myDomain;
    }

    vector<string> url;

    GravityReturnCode ret;
    ret = ServiceDirectoryDataProductLookup(dataProductID, url, domain);
    if(ret != GravityReturnCodes::SUCCESS)
        return ret;

    if (url.size() == 0)
    {
        subscribe("", dataProductID, subscriber, filter, domain);
    }
    else
    {
        // Subscribe to all published data products
        for (size_t i = 0; i < url.size(); i++)
        {
            subscribe(url[i], dataProductID, subscriber, filter, domain);
        }
    }

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::subscribe(string connectionURL, string dataProductID,
        const GravitySubscriber& subscriber, string filter, string domain)
{
	vector<string> url;
	GravityReturnCode ret;
	int tries = 5;
	while (url.size() == 0 && tries-- > 0)
	{
		ret = ServiceDirectoryDataProductLookup("RegisteredPublishers", url, myDomain);
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
	sendStringMessage(subscriptionManagerSWL.socket, "subscribe", ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, connectionURL, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, filter, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, domain, ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, url[0], ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(&subscriber));
	void* v = (void*)&subscriber;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&subscriber));
	zmq_sendmsg(subscriptionManagerSWL.socket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

	SubscriptionDetails details;
	details.dataProductID = dataProductID;
	details.domain = domain;
	details.filter = filter;
	details.subscriber = &subscriber;
	subscriptionList.push_back(details);

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::unsubscribe(string dataProductID, const GravitySubscriber& subscriber, string filter, string domain)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret = unsubscribeInternal(dataProductID, subscriber, filter, domain);
    subscriptionManagerSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::unsubscribeInternal(string dataProductID, const GravitySubscriber& subscriber, string filter, string domain)
{
	if (domain.empty())
	{
		domain = myDomain;
	}
	// Send unsubscribe details
	sendStringMessage(subscriptionManagerSWL.socket, "unsubscribe", ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, filter, ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, domain, ZMQ_SNDMORE);

	// Send subscriber
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(&subscriber));
	void* v = (void*)&subscriber;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&subscriber));
	zmq_sendmsg(subscriptionManagerSWL.socket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

	list<SubscriptionDetails>::iterator iter = subscriptionList.begin();
	while (iter != subscriptionList.end())
	{
	    if (iter->dataProductID == dataProductID && iter->domain == domain &&
	        iter->filter == filter && iter->subscriber == &subscriber)
	    {
	        iter = subscriptionList.erase(iter);
	    }
	    else
	    {
	        ++iter;
	    }
	}

	return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::publish(const GravityDataProduct& dataProduct, std::string filterText, uint64_t timestamp)
{
    string dataProductID = dataProduct.getDataProductID();

    //Set Timestamp
    if (timestamp == 0)
    {
        dataProduct.setTimestamp(getCurrentTime());
    }
    else
    {
        dataProduct.setTimestamp(timestamp);
    }

	//set Component ID
	dataProduct.setComponentId(componentID);

	//set Domain
	dataProduct.setDomain(myDomain);

	// Send subscription details
    publishManagerPublishSWL.lock.Lock();

    sendStringMessage(publishManagerPublishSWL.socket, "publish", ZMQ_SNDMORE);
    sendStringMessage(publishManagerPublishSWL.socket, dataProductID, ZMQ_SNDMORE);
    sendUint64Message(publishManagerPublishSWL.socket, dataProduct.getGravityTimestamp(), ZMQ_SNDMORE);
	sendStringMessage(publishManagerPublishSWL.socket, filterText, ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&msg));
	zmq_sendmsg(publishManagerPublishSWL.socket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    publishManagerPublishSWL.lock.Unlock();

    return GravityReturnCodes::SUCCESS;
}

/**
 * Used to re-register if we see that the ServiceDirectory has restarted.
 */
GravityReturnCode GravityNode::ServiceDirectoryReregister(string componentId)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    Log::warning("ServiceDirectory restart detected, attempting to re-register...");

    // GravityPublishManager already has this info, so just need to update the ServiceDirectory
    publishManagerRequestSWL.lock.Lock();
    for (map<string, string>::const_iterator iter = publishMap.begin(); iter != publishMap.end(); ++iter)
    {
        ServiceDirectoryRegistrationPB registration;
        registration.set_id(iter->first);
        registration.set_url(iter->second);
        registration.set_type(ServiceDirectoryRegistrationPB::DATA);
        registration.set_component_id(componentId);
		registration.set_timestamp(urlInstanceMap[iter->second]);

        // Wrap request in GravityDataProduct
        GravityDataProduct request("RegistrationRequest");
        request.setData(registration);

        // GravityDataProduct for response
        GravityDataProduct response("RegistrationResponse");

        // Send request to service directory
        GravityReturnCode pubRet = sendRequestToServiceDirectory(request, response);
        int numTries = 3;
        while (pubRet != GravityReturnCodes::SUCCESS && numTries-- > 0)
        {
            Log::debug("Error re-registering publisher, retrying...");
            pubRet = sendRequestToServiceDirectory(request, response);
        }
        if (pubRet == GravityReturnCodes::SUCCESS)
        {
            Log::message("Successfully re-registered data product %s", iter->first.c_str());
        }
        else
        {
            Log::critical("Error re-registering data product %s: %s", iter->first.c_str(), getCodeString(pubRet).c_str());
            ret = pubRet;
        }
    }
    publishManagerRequestSWL.lock.Unlock();

    // GravityServiceManager already has this info, so just need to update the ServiceDirectory
    serviceManagerSWL.lock.Lock();
    for (map<string, string>::const_iterator iter = serviceMap.begin(); iter != serviceMap.end(); ++iter)
    {
        ServiceDirectoryRegistrationPB registration;
        registration.set_id(iter->first);
        registration.set_url(iter->second);
        registration.set_type(ServiceDirectoryRegistrationPB::SERVICE);
        registration.set_component_id(componentId);
		registration.set_timestamp(urlInstanceMap[iter->second]);

        // Wrap request in GravityDataProduct
        GravityDataProduct request("RegistrationRequest");
        request.setData(registration);

        // GravityDataProduct for response
        GravityDataProduct response("RegistrationResponse");

        // Send request to service directory
        GravityReturnCode servRet = sendRequestToServiceDirectory(request, response);
        int numTries = 3;
        while (servRet != GravityReturnCodes::SUCCESS && numTries-- > 0)
        {
            Log::debug("Error re-registering service, retrying...");
            servRet = sendRequestToServiceDirectory(request, response);
        }
        if (servRet == GravityReturnCodes::SUCCESS)
        {
            Log::message("Successfully re-registered service %s", iter->first.c_str());
        }
        else
        {
            Log::critical("Error re-registering service %s: %s", iter->first.c_str(), getCodeString(servRet).c_str());
            ret = servRet;
        }
    }
    serviceManagerSWL.lock.Unlock();

    subscriptionManagerSWL.lock.Lock();

    // Make a copy since subscriptionList will be updated as we re-subscribe to this list
    list<SubscriptionDetails> origList = subscriptionList;

    for (list<SubscriptionDetails>::const_iterator iter = origList.begin(); iter != origList.end(); ++iter)
    {
        GravityReturnCode subRet = unsubscribeInternal(iter->dataProductID, *iter->subscriber, iter->filter, iter->domain);
        int numTries = 3;
        while (subRet != GravityReturnCodes::SUCCESS && numTries-- > 0)
        {
            Log::debug("Error cleaning up old subscription before re-subscribing, retrying...");
            subRet = unsubscribeInternal(iter->dataProductID, *iter->subscriber, iter->filter, iter->domain);
        }

        if (subRet == GravityReturnCodes::SUCCESS)
        {
            subRet = subscribeInternal(iter->dataProductID, *iter->subscriber, iter->filter, iter->domain);
            numTries = 3;
            while (subRet != GravityReturnCodes::SUCCESS && numTries-- > 0)
            {
                Log::debug("Error re-subscribing, retrying...");
                subRet = subscribeInternal(iter->dataProductID, *iter->subscriber, iter->filter, iter->domain);
            }
            Log::message("Successfully re-subscribed %s", iter->dataProductID.c_str());
        }
        else
        {
            Log::critical("Error re-registering service %s: %s", iter->dataProductID.c_str(), getCodeString(subRet).c_str());
            ret = subRet;
        }
    }
    subscriptionManagerSWL.lock.Unlock();

    if (ret == GravityReturnCodes::SUCCESS)
        Log::warning("Successfully re-registered with the ServiceDirectory");

    return ret;
}

GravityReturnCode GravityNode::ServiceDirectoryServiceLookup(std::string serviceID, std::string &url, string &domain)
{
	// Create the object describing the data product to lookup
	ComponentLookupRequestPB lookup;
	lookup.set_lookupid(serviceID);
	lookup.set_domain_id(domain);
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
        const GravityRequestor& requestor, string requestID, int timeout_milliseconds, string domain)
{
	std::string url;
	GravityReturnCode ret = ServiceDirectoryServiceLookup(serviceID, url, domain);
	if(ret != GravityReturnCodes::SUCCESS)
		return ret;
	return request(url, serviceID, dataProduct, requestor, requestID, timeout_milliseconds);
}

//Asynchronous Request with URL
GravityReturnCode GravityNode::request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
        const GravityRequestor& requestor, string requestID, int timeout_milliseconds)
{
	// Send subscription details
    requestManagerSWL.lock.Lock();

	//set Component ID
	dataProduct.setComponentId(componentID);

	//set Domain
	dataProduct.setDomain(myDomain);

	sendStringMessage(requestManagerSWL.socket, "request", ZMQ_SNDMORE);
	sendStringMessage(requestManagerSWL.socket, serviceID, ZMQ_SNDMORE);
	sendStringMessage(requestManagerSWL.socket, connectionURL, ZMQ_SNDMORE);
	sendStringMessage(requestManagerSWL.socket, requestID, ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, dataProduct.getSize());
	dataProduct.serializeToArray(zmq_msg_data(&msg));
	zmq_sendmsg(requestManagerSWL.socket, &msg, ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	zmq_msg_init_size(&msg, sizeof(&requestor));
	void* v = (void*)&requestor;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&requestor));
	zmq_sendmsg(requestManagerSWL.socket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    requestManagerSWL.lock.Unlock();

	return GravityReturnCodes::SUCCESS;
}

//Synchronous Request
shared_ptr<GravityDataProduct> GravityNode::request(string serviceID, const GravityDataProduct& request, 
													int timeout_milliseconds, string domain)
{
	//set Component ID
	request.setComponentId(componentID);

	//set Domain
	request.setDomain(myDomain);

	Log::trace("Synchronous request('%s','%s')", serviceID.c_str(), request.getDataProductID().c_str());

	std::string connectionURL;
	GravityReturnCode ret = ServiceDirectoryServiceLookup(serviceID, connectionURL, domain);
	if(ret != GravityReturnCodes::SUCCESS)
	{
		//special case for ConfigService. Since this happens frequently, log message not warning.
		if (serviceID=="ConfigService")
		{
			Log::message("Unable to find service %s: %s", serviceID.c_str(), getCodeString(ret).c_str());
		}
		else
		{
			Log::warning("Unable to find service %s: %s", serviceID.c_str(), getCodeString(ret).c_str());
		}
		return shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
	}

	uint64_t t1 = gravity::getCurrentTime();
	shared_ptr<GravityDataProduct> response(new GravityDataProduct(serviceID));
	Log::trace("Sending request to service provider @ %s", connectionURL.c_str());
	ret = sendRequestToServiceProvider(connectionURL, request, *response, timeout_milliseconds);
	if(ret != GravityReturnCodes::SUCCESS)
	{
		Log::warning("service request returned error: %s", getCodeString(ret).c_str());
		return shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
	}

	if (response->isFutureResponse())
	{
		Log::trace("Synchronous Request: received a future response");
		if (timeout_milliseconds > 0)
		{
			timeout_milliseconds -= (int)((gravity::getCurrentTime() - t1) / 1e3);
			if (timeout_milliseconds <= 0)
			{
				timeout_milliseconds = 1;
			}
		}
		Log::trace("Sending request to future response socket (url='%s', timeout=%d)", response->getFutureSocketUrl().c_str(), timeout_milliseconds);
		ret = sendRequestToServiceProvider(response->getFutureSocketUrl(), request, *response, timeout_milliseconds);
		if(ret != GravityReturnCodes::SUCCESS)
		{
			Log::warning("service request returned error: %s", getCodeString(ret).c_str());
			return shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
		}
		Log::trace("Received future response's response");
	}

	return response;
}

GravityReturnCode GravityNode::registerService(string serviceID, GravityTransportType transportType,
        const GravityServiceProvider& server)
{
    string transportType_str;

    // manage access to service manager socket as well as serviceMap.
    serviceManagerSWL.lock.Lock();
    if (serviceMap.count(serviceID) > 0)
    {
        Log::warning("attempt to register duplicate service ID: %s", serviceID.c_str());
        serviceManagerSWL.lock.Unlock();
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

	sendStringMessage(serviceManagerSWL.socket, "register", ZMQ_SNDMORE);
	sendStringMessage(serviceManagerSWL.socket, serviceID, ZMQ_SNDMORE);
	sendStringMessage(serviceManagerSWL.socket, transportType_str, ZMQ_SNDMORE);

    if(transportType == GravityTransportTypes::TCP)
    {
        int minPort = getIntParam("MinPort", MIN_PORT);
        int maxPort = getIntParam("MaxPort", MAX_PORT);
        sendIntMessage(serviceManagerSWL.socket, minPort, ZMQ_SNDMORE);
        sendIntMessage(serviceManagerSWL.socket, maxPort, ZMQ_SNDMORE);
    }
    sendStringMessage(serviceManagerSWL.socket, endpoint, ZMQ_SNDMORE);


	// Include the server
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(&server));
	void* v = (void*)&server;
	memcpy(zmq_msg_data(&msg), &v, sizeof(&server));
	zmq_sendmsg(serviceManagerSWL.socket, &msg, ZMQ_DONTWAIT);
	zmq_msg_close(&msg);

    string connectionURL = readStringMessage(serviceManagerSWL.socket);
	uint64_t timestamp = getCurrentTime();

    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    if (ret == GravityReturnCodes::SUCCESS && !serviceDirectoryNode.ipAddress.empty())
    {
        // Create the object describing the data product to register
        ServiceDirectoryRegistrationPB registration;
        registration.set_id(serviceID);
        registration.set_url(connectionURL);
        registration.set_type(ServiceDirectoryRegistrationPB::SERVICE);
        registration.set_component_id(componentID);
		registration.set_timestamp(timestamp);

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
        sendStringMessage(serviceManagerSWL.socket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(serviceManagerSWL.socket, serviceID, ZMQ_DONTWAIT);
        // wait for it to return
        readStringMessage(serviceManagerSWL.socket);
    }
    else
    {
        Log::debug("Registered service at address: %s", connectionURL.c_str());
        serviceMap[serviceID] = connectionURL;
		urlInstanceMap[connectionURL] = timestamp;
    }
    serviceManagerSWL.lock.Unlock();
    return ret;
}

GravityReturnCode GravityNode::unregisterService(string serviceID)
{
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;
    serviceManagerSWL.lock.Lock();
    if (serviceMap.count(serviceID) == 0)
    {
        ret = GravityReturnCodes::REGISTRATION_CONFLICT;
    }
    else
    {
        sendStringMessage(serviceManagerSWL.socket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(serviceManagerSWL.socket, serviceID, ZMQ_DONTWAIT);
        string url = serviceMap[serviceID];
        serviceMap.erase(serviceID);
		urlInstanceMap.erase(url);

        string status = readStringMessage(serviceManagerSWL.socket);

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
    serviceManagerSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::startHeartbeat(int64_t interval_in_microseconds)
{
	if(interval_in_microseconds < 0)
		return gravity::GravityReturnCodes::FAILURE;

	if(heartbeatStarted)
		return gravity::GravityReturnCodes::FAILURE; //We shouldn't be able to start this guy twice

	std::string heartbeatName;
	//Gravity Heartbeats should be keyed with the domain
	heartbeatName = componentID+"_GravityHeartbeat_"+myDomain;

	this->registerDataProduct(heartbeatName, GravityTransportTypes::TCP);

	HBParams* params = new HBParams(); //(freed by thread)
	params->zmq_context = context;
	params->interval_in_microseconds = interval_in_microseconds;
	params->componentID = heartbeatName;
	params->minPort = getIntParam("MinPort", MIN_PORT);
	params->maxPort = getIntParam("MaxPort", MAX_PORT);
	params->endpoint = getIP();

	pthread_t heartbeatThread;
	pthread_create(&heartbeatThread, NULL, Heartbeat, (void*)params);

	heartbeatStarted=true;

	return gravity::GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::registerHeartbeatListener(string componentID, int64_t timebetweenMessages, 
									const GravityHeartbeatListener& listener, string domain)
{
	void* HeartbeatListener(void*); //Forward declaration.
	static class Heartbeat hbSub;
	GravityReturnCode ret = GravityReturnCodes::SUCCESS;
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

	std::string heartbeatName;

	//Gravity Heartbeats should be keyed with the domain
	heartbeatName = componentID+"_GravityHeartbeat_"+(domain.empty()?myDomain:domain);

	ret = this->subscribe(heartbeatName, hbSub,"",domain);

	if (ret==GravityReturnCodes::SUCCESS)
	{
		//Send the DataproductID
		sendStringMessage(hbSocket, "register", ZMQ_SNDMORE);
		sendStringMessage(hbSocket, heartbeatName, ZMQ_SNDMORE);

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
		zmq_sendmsg(hbSocket, &msg, ZMQ_DONTWAIT);
		zmq_msg_close(&msg);

		// Read the ACK
		readStringMessage(hbSocket);
	}

	return ret;
}

GravityReturnCode GravityNode::unregisterHeartbeatListener(string componentID, string domain)
{
	static class Heartbeat hbSub;
	
	std::string heartbeatName;
	heartbeatName = componentID+"_GravityHeartbeat_"+(domain.empty()?myDomain:domain);

	this->unsubscribe(heartbeatName,hbSub);

	//Send the DataproductID
	sendStringMessage(hbSocket,"unregister",ZMQ_SNDMORE);
	sendStringMessage(hbSocket, heartbeatName, ZMQ_DONTWAIT);

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

string GravityNode::getDomain()
{
    return myDomain;
}

shared_ptr<FutureResponse> GravityNode::createFutureResponse()
{
	// Send request to create future response
    requestManagerRepSWL.lock.Lock();

	// Get data required for socket creation
	string ip = getIP();
	int minPort = getIntParam("MinPort", MIN_PORT);
    int maxPort = getIntParam("MaxPort", MAX_PORT);

	// Send request for FutureResponse and info required to create socket
	sendStringMessage(requestManagerRepSWL.socket, "createFutureResponse", ZMQ_SNDMORE);
	sendStringMessage(requestManagerRepSWL.socket, ip, ZMQ_SNDMORE);
	sendIntMessage(requestManagerRepSWL.socket, minPort, ZMQ_SNDMORE);
	sendIntMessage(requestManagerRepSWL.socket, maxPort, ZMQ_DONTWAIT);

	// Get URL for FutureResponse from GravityRequestManager
	string url = readStringMessage(requestManagerRepSWL.socket, 0);

	if (url.empty())
	{
		Log::critical("Could not find available port for FutureResponse");
		return shared_ptr<FutureResponse>((FutureResponse*)NULL);
	}

	requestManagerRepSWL.lock.Unlock();

	shared_ptr<FutureResponse> futureResponse(new FutureResponse(url));
	return futureResponse;
}

GravityReturnCode GravityNode::sendFutureResponse(const FutureResponse& futureResponse)
{
	requestManagerRepSWL.lock.Lock();
	
	sendStringMessage(requestManagerRepSWL.socket, "sendFutureResponse", ZMQ_SNDMORE);
	sendStringMessage(requestManagerRepSWL.socket, futureResponse.getUrl(), ZMQ_SNDMORE);
	
	// Send the reponse object
	int size = futureResponse.getDataSize();
	char *bytes = new char[size];		
	futureResponse.getData(bytes, size);
	GravityDataProduct response(bytes, size);
	response.setComponentId(componentID);
	response.setDomain(myDomain);
	sendGravityDataProduct(requestManagerRepSWL.socket, response, ZMQ_DONTWAIT);
	delete bytes;

	// Get results back from GravityRequestManager
	int ret = readIntMessage(requestManagerRepSWL.socket);
	
	requestManagerRepSWL.lock.Unlock();

	return static_cast<GravityReturnCode>(ret);
}

GravityReturnCode GravityNode::setSubscriptionTimeoutMonitor(string dataProductID, const GravitySubscriptionMonitor& monitor, 
			int milliSecondTimeout, string filter, string domain)
{
	if (domain.empty())
    {
        domain = myDomain;
    }

	if(milliSecondTimeout <= 0)
	{
		return GravityReturnCodes::INVALID_PARAMETER;
	}

	subscriptionManagerSWL.lock.Lock();

	sendStringMessage(subscriptionManagerSWL.socket, "set_monitor", ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);
	
	//Send the address of the monitor
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(GravitySubscriptionMonitor*));
	intptr_t p = (intptr_t) &monitor;
	memcpy(zmq_msg_data(&msg), &p, sizeof(GravitySubscriptionMonitor*));
	zmq_sendmsg(subscriptionManagerSWL.socket, &msg, ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	//Send the Max time between messages
	sendIntMessage(subscriptionManagerSWL.socket,milliSecondTimeout,ZMQ_SNDMORE);

	sendStringMessage(subscriptionManagerSWL.socket, filter, ZMQ_SNDMORE);

	sendStringMessage(subscriptionManagerSWL.socket, domain, ZMQ_DONTWAIT);

	//int ret = readIntMessage(subscriptionManagerSWL.socket);
	
	subscriptionManagerSWL.lock.Unlock();

	return GravityReturnCodes::SUCCESS;

}

GravityReturnCode GravityNode::clearSubscriptionTimeoutMonitor(std::string dataProductID, const GravitySubscriptionMonitor& monitor, 
			string filter, string domain)
{
	subscriptionManagerSWL.lock.Lock();

	sendStringMessage(subscriptionManagerSWL.socket, "clear_monitor", ZMQ_SNDMORE);
	sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);
	
	//Send the address of the monitor
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, sizeof(GravitySubscriptionMonitor*));
	intptr_t p = (intptr_t) &monitor;
	memcpy(zmq_msg_data(&msg), &p, sizeof(GravitySubscriptionMonitor*));
	zmq_sendmsg(subscriptionManagerSWL.socket, &msg, ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	sendStringMessage(subscriptionManagerSWL.socket, filter, ZMQ_SNDMORE);

	sendStringMessage(subscriptionManagerSWL.socket, domain, ZMQ_DONTWAIT);

	//int ret = readIntMessage(subscriptionManagerSWL.socket);
	
	subscriptionManagerSWL.lock.Unlock();

	return GravityReturnCodes::SUCCESS;
}

string GravityNode::getIP()
{
    string ip = "127.0.0.1";

    serviceDirectoryLock.Lock();

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

        free(buffer);
    }

    serviceDirectoryLock.Unlock();

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

std::string GravityNode::getComponentID()
{
    return componentID;
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
