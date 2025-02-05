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
#include <thread>
#include <assert.h>
#ifdef WIN32
#include <winsock2.h>
#include <WinBase.h>
#include <Windows.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif
#include <sstream>
#include <signal.h>
#include <memory>
#include <cmath>

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
#include "protobuf/GravityConfigParamPB.pb.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "PublishSink.h"
#include <mutex>

#include "GravityNode.h"  //Needs to be last on Windows so it is included after nb30.h for the DUPLICATE definition.

#ifdef WIN32
const std::string gravity::GravityNode::file_separator = "\\";
#else
const std::string gravity::GravityNode::file_separator = "/";
#endif

using proxy_dist_sink_mt = gravity::proxy_dist_sink<std::mutex>;

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
static void (*previousHandlerAbrt)(int);  //Function Pointer
static void (*previousHandlerInt)(int);   //Function Pointer
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

GravityNode::GravityNodeDomainListener::GravityNodeDomainListener(void* context)
{
    this->context = context;
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
    ready = false;

    gravityNodeSocket = zmq_socket(context, ZMQ_REP);
    zmq_connect(gravityNodeSocket, "inproc://gravity_domain_listener");
    zmq_setsockopt(gravityNodeSocket, ZMQ_SUBSCRIBE, NULL, 0);

    void* domainSocket = zmq_socket(context, ZMQ_PUB);
    zmq_connect(domainSocket, "inproc://gravity_domain_receiver");

    // Poll the gravity node
    zmq_pollitem_t pollItem;
    pollItem.socket = gravityNodeSocket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;

    shared_ptr<spdlog::logger> logger = spdlog::get("GravityLogger");

    while (!ready)
    {
        // Start polling socket(s), blocking while we wait
        int rc = zmq_poll(&pollItem, 1, -1);  // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            logger->debug("GravityNode zmq_poll error, exiting (errno = {})", errno);
            break;
        }

        // Process new subscription requests from the gravity node
        if (pollItem.revents & ZMQ_POLLIN)
        {
            std::string command = readStringMessage(gravityNodeSocket);
            if (command == "configure")
            {
                readDomainListenerParameters();
                ready = true;
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
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));  /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    broadcastAddr.sin_port = htons(port);              /* Broadcast port */

    //set socket to be re-usable. Must be set for all other listeners for this port
    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));

    /* Bind to the broadcast port */
#ifdef _WIN32
    int rc = bind((SOCKET)sock, (const struct sockaddr*)&broadcastAddr, (int)sizeof(broadcastAddr));
#else
    int rc = bind(sock, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
#endif
    if (rc < 0)
    {
        return;
    }

    char recvString[MAXRECVSTRING + 1]; /* Buffer for received string */
    int recvStringLen;                  /* Length of received string */
    ServiceDirectoryBroadcastPB broadcastPB;

    struct timeval timetowait;
    timetowait.tv_sec = timeout;
    timetowait.tv_usec = 0;

    //set socket to block forever
#ifdef _WIN32
    int timeout_int = timevalToMilliSeconds(&timetowait);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_int, sizeof(unsigned int));
#else
    //set socket to block forever initially
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timetowait, sizeof(struct timeval));
#endif

    running = true;

    while (running)
    {
        //wait for braodcast message to be recieved
        memset(recvString, 0, MAXRECVSTRING + 1);

        /* Receive a broadcast message or timeout */
        recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0);

        //check for socket error
        if (recvStringLen < 0)
        {
            //if we received some error
            int error = errno;
            if (error == EAGAIN)
                logger->trace("Timed out waiting for domain broadcast");
            else
                logger->warn("Received error reading domain listener socket: %d", error);
        }
        else  //we received a message
        {
            broadcastPB.ParseFromArray(recvString, recvStringLen);
            //logger->trace("Domain listener received message from domain '{}'", broadcastPB.domain());

            //if the domains match
            if (domain.compare(broadcastPB.domain()) == 0)
            {
                sendStringMessage(domainSocket, "connect", ZMQ_SNDMORE);
                sendStringMessage(domainSocket, domain, ZMQ_SNDMORE);
                sendStringMessage(domainSocket, broadcastPB.url(), ZMQ_DONTWAIT);

                // if it's a new start time...
                if (serviceDirectoryStartTime < broadcastPB.starttime())
                {
                    logger->debug(
                        "Domain listener found update to our domain, orig SD start time = {}, new SD start time = {}, "
                        "SD url is now {}",
                        serviceDirectoryStartTime, broadcastPB.starttime(), broadcastPB.url());
                    // If we've seen a start time before, then re-register
                    if (serviceDirectoryStartTime != 0)
                    {
                        gravityNode->ServiceDirectoryReregister(compId, broadcastPB.url());
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
            sendStringMessage(gravityNodeSocket, "ACK", ZMQ_DONTWAIT);
            if (command == "kill")
            {
                running = false;
                break;
            }
        }

        //don't set a timeout on the socket if it has already been closed
        if (running)
        {
//set new timeout
#ifdef _WIN32
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_int, sizeof(unsigned int));
#else
            //set socket to block until the timeout
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timetowait, sizeof(struct timeval));
#endif
        }

    }  //end while

    logger->warn("Closing Domain Receiver");

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
    domain = readStringMessage(gravityNodeSocket);

    //receive port
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(gravityNodeSocket, &msg, ZMQ_DONTWAIT);
    memcpy(&port, zmq_msg_data(&msg), sizeof(unsigned int));
    zmq_msg_close(&msg);

    //receive timeout
    zmq_msg_t msg2;
    zmq_msg_init(&msg2);
    zmq_recvmsg(gravityNodeSocket, &msg2, ZMQ_DONTWAIT);
    memcpy(&timeout, zmq_msg_data(&msg2), sizeof(unsigned int));
    zmq_msg_close(&msg2);

    //receive component ID
    compId = readStringMessage(gravityNodeSocket);

    //receive GravityNode pointer
    zmq_msg_t msg3;
    zmq_msg_init(&msg3);
    zmq_recvmsg(gravityNodeSocket, &msg3, ZMQ_DONTWAIT);
    memcpy(&gravityNode, zmq_msg_data(&msg3), sizeof(GravityNode*));
    zmq_msg_close(&msg3);

    sendStringMessage(gravityNodeSocket, "ACK", ZMQ_DONTWAIT);
}

Semaphore GravityNode::initLock;
GravityNode::GravityNode()
{
    // Populating (ServiceDirectory) set of reserved data product IDs
    serviceDirectory_ReservedDataProductIDs.insert(gravity::constants::REGISTERED_PUBLISHERS_DPID);
    serviceDirectory_ReservedDataProductIDs.insert(gravity::constants::DOMAIN_DETAILS_DPID);
    serviceDirectory_ReservedDataProductIDs.insert(gravity::constants::DOMAIN_UPDATE_DPID);
    serviceDirectory_ReservedDataProductIDs.insert(gravity::constants::DIRECTORY_SERVICE_DPID);

    // Populating (GravityNode) set of reserved data product IDs
    gravityNode_ReservedDataProductIDs.insert(gravity::constants::METRICS_DATA_DPID);
    gravityNode_ReservedDataProductIDs.insert(gravity::constants::GRAVITY_SETTINGS_DPID);
    gravityNode_ReservedDataProductIDs.insert(gravity::constants::GRAVITY_LOGGER_DPID);

    defaultReceiveLastSentDataproduct = true;
    defaultCacheLastSentDataprodut = true;

    // Eventually to be read from a config/properties file
    serviceDirectoryNode.port = 5555;
    serviceDirectoryNode.transport = "tcp";
    serviceDirectoryNode.socket = NULL;

    //Initialize this guy so we can know whether the heartbeat thread has started.
    hbSocket = NULL;

    // Default to no metrics
    metricsEnabled = false;
    settingsPubEnabled = false;
    initialized = false;
    logInitialized = false;
    listenerEnabled = false;
    heartbeatStarted = false;

    parser = NULL;
}

GravityNode::GravityNode(std::string componentID)
{
    componentID = "";

    // Eventually to be read from a config/properties file
    serviceDirectoryNode.port = 5555;
    serviceDirectoryNode.transport = "tcp";
    serviceDirectoryNode.socket = NULL;

    //Initialize this guy so we can know whether the heartbeat thread has started.
    hbSocket = NULL;

    // Default to no metrics
    metricsEnabled = false;
    settingsPubEnabled = false;
    initialized = false;
    logInitialized = false;
    heartbeatStarted = false;

    parser = NULL;

    init(componentID);
}

GravityNode::~GravityNode()
{
    // If metrics are enabled, we need to unregister our metrics data product
    if (metricsEnabled)
    {
        unregisterDataProduct(gravity::constants::METRICS_DATA_DPID);
    }

    //kill the domain listener
    if (listenerEnabled)
    {
        if (domainListenerSWL.socket)
        {
            sendStringMessage(domainListenerSWL.socket, "kill", ZMQ_DONTWAIT);
            readStringMessage(domainListenerSWL.socket);
            zmq_close(domainListenerSWL.socket);
        }
        if (domainRecvSWL.socket)
        {
            zmq_close(domainRecvSWL.socket);
        }
    }

    // Close the inproc sockets
    if (subscriptionManagerSWL.socket)
    {
        sendStringMessage(subscriptionManagerSWL.socket, "kill", ZMQ_DONTWAIT);
        zmq_close(subscriptionManagerSWL.socket);
    }

    if (subscriptionManagerConfigSWL.socket)
    {
        zmq_close(subscriptionManagerConfigSWL.socket);
    }

    if (requestManagerSWL.socket)
    {
        sendStringMessage(requestManagerSWL.socket, "kill", ZMQ_DONTWAIT);
        zmq_close(requestManagerSWL.socket);
    }

    if (requestManagerRepSWL.socket)
    {
        zmq_close(requestManagerRepSWL.socket);
    }

    if (publishManagerPublishSWL.socket)
    {
        sendStringMessage(publishManagerPublishSWL.socket, "kill", ZMQ_DONTWAIT);
        zmq_close(publishManagerPublishSWL.socket);
    }

    if (publishManagerRequestSWL.socket)
    {
        zmq_close(publishManagerRequestSWL.socket);
    }

    if (serviceManagerSWL.socket)
    {
        sendStringMessage(serviceManagerSWL.socket, "kill", ZMQ_DONTWAIT);
        zmq_close(serviceManagerSWL.socket);
    }

    if (serviceManagerConfigSWL.socket)
    {
        zmq_close(serviceManagerConfigSWL.socket);
    }

    if (metricsManagerSocket)
    {
        sendStringMessage(metricsManagerSocket, "kill", ZMQ_DONTWAIT);
        zmq_close(metricsManagerSocket);
    }

    // Clean up heartbeat listener
    if (hbSocket)
    {
        sendStringMessage(hbSocket, "kill", ZMQ_DONTWAIT);
        zmq_close(hbSocket);
    }

    // Clean up heartbeat publisher
    if (heartbeatStarted)
    {
        stopHeartbeat();
    }

    // Clean up the zmq context object
    if (context)
    {
        zmq_term(context);
    }

    if (parser)
    {
        delete parser;
    }

    //Do not destroy object until sub manager thread is joined
    if (subscriptionManagerThread.joinable())
    {
        subscriptionManagerThread.join();
    }
}

void GravityNode::configSpdLoggers()
{
    if (logger = spdlog::get("GravityLogger"))
    {
        return;
    }

    // Get log levels from INI file
    auto gravity_file_level = spdlog::level::from_str(StringToLowerCase(getStringParam("GravityFileLogLevel", "off")));
    auto gravity_console_level =
        spdlog::level::from_str(StringToLowerCase(getStringParam("GravityConsoleLogLevel", "off")));
    auto app_file_level = spdlog::level::from_str(StringToLowerCase(getStringParam("AppFileLogLevel", "off")));
    auto app_console_level = spdlog::level::from_str(StringToLowerCase(getStringParam("AppConsoleLogLevel", "off")));
    auto app_publish_level = spdlog::level::from_str(StringToLowerCase(getStringParam("AppNetworkLogLevel", "off")));

    bool has_gravity_file_logger = gravity_file_level != SPDLOG_LEVEL_OFF;
    bool has_app_file_logger = app_file_level != SPDLOG_LEVEL_OFF;
    bool has_app_console_logger = app_console_level != SPDLOG_LEVEL_OFF;
    bool has_app_publish_logger = app_publish_level != SPDLOG_LEVEL_OFF;

    bool has_app_logger = has_app_file_logger || has_app_console_logger || has_app_publish_logger;

    // Create lists to hold sinks
    std::list<shared_ptr<spdlog::sinks::sink>> app_sink_list = {};
    std::list<shared_ptr<spdlog::sinks::sink>> gravity_sink_list = {};

    // Always create console loggers
    auto shared_console_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto console_proxy_for_gravity = std::make_shared<proxy_dist_sink_mt>();
    console_proxy_for_gravity->add_sink(shared_console_sink);
    console_proxy_for_gravity->set_level(gravity_console_level);
    gravity_sink_list.push_back(console_proxy_for_gravity);
    auto console_proxy_for_app = std::make_shared<proxy_dist_sink_mt>();
    console_proxy_for_app->add_sink(shared_console_sink);
    console_proxy_for_app->set_level(app_console_level);
    app_sink_list.push_back(console_proxy_for_app);

    // Configure file logger (if specified)
    if (has_gravity_file_logger || has_app_file_logger)
    {
        string filename = getStringParam("LogDirectory", ".") + file_separator + componentID + ".log";
        auto sharedFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);

        if (has_gravity_file_logger)
        {
            auto file_proxy_for_gravity = std::make_shared<proxy_dist_sink_mt>();
            file_proxy_for_gravity->add_sink(sharedFileSink);
            file_proxy_for_gravity->set_level(gravity_file_level);
            gravity_sink_list.push_back(file_proxy_for_gravity);
        }

        if (has_app_file_logger)
        {
            auto file_proxy_for_app = std::make_shared<proxy_dist_sink_mt>();
            file_proxy_for_app->add_sink(sharedFileSink);
            file_proxy_for_app->set_level(app_file_level);
            app_sink_list.push_back(file_proxy_for_app);
        }
    }

    // Configure publish logger
    if (has_app_publish_logger)
    {
        auto publishSink = std::make_shared<PublishSink<std::mutex>>(this);
        auto publish_proxy_for_app = std::make_shared<proxy_dist_sink_mt>();
        publish_proxy_for_app->add_sink(publishSink);
        publish_proxy_for_app->set_level(app_publish_level);
        app_sink_list.push_back(publish_proxy_for_app);
    }

    // Configure GravityLogger.
    logger = std::make_shared<spdlog::logger>("GravityLogger");
    for (auto sink : gravity_sink_list)
    {
        logger->sinks().push_back(sink);
    }
    logger->set_level(spdlog::level::trace);  // logger will pass through all logs to be filtered by sinks
    logger->flush_on(spdlog::level::trace);   // logger will flush on all messages
    logger->set_pattern("[%Y-%m-%d %T.%f " + componentID + "-%l] %v");
    spdlog::register_logger(logger);

    // Configure ApplicationLogger
    auto app_logger = std::make_shared<spdlog::logger>("GravityApplicationLogger");
    for (auto sink : app_sink_list)
    {
        app_logger->sinks().push_back(sink);
    }
    app_logger->set_level(spdlog::level::trace);  // logger will pass through all logs to be filtered by sinks
    app_logger->flush_on(spdlog::level::trace);   // logger will flush on all messages
    app_logger->set_pattern("[%Y-%m-%d %T.%f " + componentID + "-%l] %v");
    spdlog::register_logger(app_logger);

    if (has_app_logger)
    {
        // Set the ApplicaitonLogger as the default
        spdlog::set_default_logger(app_logger);
    }
}

GravityReturnCode GravityNode::init()
{
    ////////////////////////////////////////////////////////
    //get gravity configuration.
    parser = new GravityConfigParser("");

    parser->ParseConfigFile("Gravity.ini");

    std::string id = parser->getString("GravityComponentID", "");

    if (id != "")
    {
        return init(id);
    }
    else
    {
        componentID = "GravityNode";
        //Setup Logging if enabled.
        if (!logInitialized)
        {
            Log::LogLevel local_log_level = Log::LogStringToLevel(getStringParam("LocalLogLevel", "none").c_str());
            if (local_log_level != Log::NONE)
                Log::initAndAddFileLogger(getStringParam("LogDirectory", "").c_str(), componentID.c_str(),
                                          local_log_level, getBoolParam("CloseLogFileAfterWrite", false));

            Log::LogLevel console_log_level = Log::LogStringToLevel(getStringParam("ConsoleLogLevel", "none").c_str());
            if (console_log_level != Log::NONE) Log::initAndAddConsoleLogger(componentID.c_str(), console_log_level);

            //log an error indicating the componentID was missing
            logger->error(
                "Field 'GravityComponentID' missing from Gravity.ini, using GravityComponentID='GravityNode'");

            logInitialized = true;
        }
        return init(componentID);
    }
}

GravityReturnCode GravityNode::init(std::string componentID)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;
    this->componentID = componentID;

    std::string serviceDirectoryUrl = "";
    bool domainTimeout = false;
    bool iniWarning = false;

    gravityNode_ReservedDataProductIDs.insert(componentID + "_GravityHeartbeat");

    initLock.Lock();

    // Setup zmq context
    if (!initialized)
    {
        ////////////////////////////////////////////////////////
        //Now that Gravity is set up, get gravity configuration.
        parser = new GravityConfigParser(componentID);

        parser->ParseConfigFile("Gravity.ini");
        std::string config_file_name = componentID + ".ini";
        if (gravity::IsValidFilename(config_file_name))
        {
            parser->ParseConfigFile(config_file_name.c_str());
        }

        // Setup Logging as soon as config parser is available.
        if (!logInitialized)
        {
            Log::LogLevel local_log_level = Log::LogStringToLevel(getStringParam("LocalLogLevel", "none").c_str());
            if (local_log_level != Log::NONE)
                Log::initAndAddFileLogger(getStringParam("LogDirectory", "").c_str(), componentID.c_str(),
                                          local_log_level, getBoolParam("CloseLogFileAfterWrite", false));

            Log::LogLevel console_log_level = Log::LogStringToLevel(getStringParam("ConsoleLogLevel", "none").c_str());
            if (console_log_level != Log::NONE) Log::initAndAddConsoleLogger(componentID.c_str(), console_log_level);

            // Configure spdlog loggers
            configSpdLoggers();

            logInitialized = true;
        }

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
        subscriptionManagerConfigSWL.socket = zmq_socket(context, ZMQ_PUB);
        zmq_bind(subscriptionManagerConfigSWL.socket, "inproc://gravity_subscription_manager_configure");

        // Setup the metrics control communication channel
        metricsManagerSocket = zmq_socket(context, ZMQ_PUB);
        zmq_bind(metricsManagerSocket, GRAVITY_METRICS_CONTROL);

        // Setup the subscription manager
        subscriptionManagerThread = std::thread(startSubscriptionManager, context);

        // Setup up publish channel to publish manager
        publishManagerPublishSWL.socket = zmq_socket(context, ZMQ_PUB);
        zmq_bind(publishManagerPublishSWL.socket, PUB_MGR_PUB_URL);

        // Setup the publish manager
        std::thread publishManagerThread(startPublishManager, context);
        publishManagerThread.detach();

        // Setup up communication channel to request manager
        requestManagerSWL.socket = zmq_socket(context, ZMQ_PUB);
        zmq_bind(requestManagerSWL.socket, "inproc://gravity_request_manager");
        requestManagerRepSWL.socket = zmq_socket(context, ZMQ_REQ);
        zmq_bind(requestManagerRepSWL.socket, "inproc://gravity_request_rep");
        // Setup the request manager
        std::thread requestManagerThread(startRequestManager, context);
        requestManagerThread.detach();

        serviceManagerConfigSWL.socket = zmq_socket(context, ZMQ_PUB);
        zmq_bind(serviceManagerConfigSWL.socket, "inproc://gravity_service_manager_configure");

        // Setup the service manager
        std::thread serviceManagerThread(startServiceManager, context);
        serviceManagerThread.detach();

        // Start the metrics manager
        std::thread metricsManagerThread(startMetricsManager, context);
        metricsManagerThread.detach();

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

        if (s_interrupted)
        {
            initLock.Unlock();
            raise(s_interrupted);
        }

        // connect down here to make sure manager has bound address.
        publishManagerRequestSWL.socket = zmq_socket(context, ZMQ_REQ);
        zmq_connect(publishManagerRequestSWL.socket, PUB_MGR_REQ_URL);

        serviceManagerSWL.socket = zmq_socket(context, ZMQ_REQ);
        zmq_connect(serviceManagerSWL.socket, SERVICE_MGR_URL);

        // Configure high water marks
        int publishHWM = getIntParam("PublishHWM", 1000);
        if (publishHWM < 0)
        {
            logger->warn("Invalid PublishHWM = {}. Ignoring.", publishHWM);
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
            logger->warn("Invalid subscribeHWM = {}. Ignoring.", subscribeHWM);
        }
        else
        {
            // Send HWM (PUB/SUB)
            sendStringMessage(subscriptionManagerSWL.socket, "set_hwm", ZMQ_SNDMORE);
            sendIntMessage(subscriptionManagerSWL.socket, subscribeHWM, ZMQ_DONTWAIT);
        }

        // Configure TCP keep-alive
        if (getBoolParam("TcpKeepAliveEnabled", false))
        {
            // Get settings
            int tcpKeepAliveTime = getIntParam("TcpKeepAliveTime", 7200);
            int tcpKeepAliveProbes = getIntParam("TcpKeepAliveProbes", 9);
            int tcpKeepAliveIntvl = getIntParam("TcpKeepAliveIntvl", 75);

            logger->info("Enabling TCP publisher Keep-Alive with settings (time={}, probes={}, intvl={}).",
                         tcpKeepAliveTime, tcpKeepAliveProbes, tcpKeepAliveIntvl);

            // Send TCP keep-alive settings
            sendStringMessage(publishManagerRequestSWL.socket, "set_tcp_keepalive", ZMQ_SNDMORE);
            sendIntMessage(publishManagerRequestSWL.socket, tcpKeepAliveTime, ZMQ_SNDMORE);
            sendIntMessage(publishManagerRequestSWL.socket, tcpKeepAliveProbes, ZMQ_SNDMORE);
            sendIntMessage(publishManagerRequestSWL.socket, tcpKeepAliveIntvl, ZMQ_DONTWAIT);

            // Read ACK
            readStringMessage(publishManagerRequestSWL.socket);
        }

        //get the Domain name of the Service Directory to connect to
        std::string serviceDirectoryDomain = getStringParam("Domain");

        //Set Service Directory URL (because we can't connect to the ConfigService without it).
        serviceDirectoryUrl = getStringParam("ServiceDirectoryURL");
        if (serviceDirectoryDomain != "" && (componentID != "ServiceDirectory"))
        {
            //if the config file specifies both domain and url
            if (serviceDirectoryUrl != "")
            {
                iniWarning = true;
            }
            //setup and start the GravityNodeDomainListener
            else
            {
                // Setup up communication channel to subscription manager
                domainListenerSWL.socket = zmq_socket(context, ZMQ_REQ);
                zmq_bind(domainListenerSWL.socket, "inproc://gravity_domain_listener");

                domainRecvSWL.socket = zmq_socket(context, ZMQ_SUB);
                zmq_bind(domainRecvSWL.socket, "inproc://gravity_domain_receiver");
                zmq_setsockopt(domainRecvSWL.socket, ZMQ_SUBSCRIBE, NULL, 0);

                std::thread domainListenerThread(startGravityDomainListener, context);
                domainListenerThread.detach();

                configureNodeDomainListener(serviceDirectoryDomain);
                int broadcastTimeout = getIntParam("ServiceDirectoryBroadcastTimeout", DEFAULT_BROADCAST_TIMEOUT_SEC);

                serviceDirectoryUrl.assign(getDomainUrl(broadcastTimeout));
                if (serviceDirectoryUrl == "")
                {
                    domainTimeout = true;
                }

                listenerEnabled = true;
            }
        }
        defaultCacheLastSentDataprodut = getBoolParam("CacheLastSentDataproduct", true);
        logger->trace("Default Setting For CacheLastSentDataproduct: {}",
                      defaultCacheLastSentDataprodut ? "TRUE" : "FALSE");

        defaultReceiveLastSentDataproduct = getBoolParam("ReceiveLastSentDataproduct", true);
        logger->trace("Default Setting For ReceiveLastSentDataproduct: {}",
                      defaultReceiveLastSentDataproduct ? "TRUE" : "FALSE");

        initialized = true;
    }
    //we are already initialzed, just try to read the SD domain and url
    else
    {
        //get the Domain name of the Service Directory to connect to
        std::string serviceDirectoryDomain = getStringParam("Domain");
        //Set Service Directory URL (because we can't connect to the ConfigService without it).
        serviceDirectoryUrl = getStringParam("ServiceDirectoryURL");

        if (serviceDirectoryDomain != "" && (componentID != "ServiceDirectory"))
        {
            //if the config file specifies both domain and url
            if (serviceDirectoryUrl != "")
            {
                iniWarning = true;
            }
            else
            {
                int broadcastTimeout = getIntParam("ServiceDirectoryBroadcastTimeout", DEFAULT_BROADCAST_TIMEOUT_SEC);

                //read the domain from the domain listener
                serviceDirectoryUrl.assign(getDomainUrl(broadcastTimeout));
                if (serviceDirectoryUrl == "")
                {
                    domainTimeout = true;
                }
            }
        }
    }

    //If we are able to proceed with trying to connect to the service directory
    if (!domainTimeout)
    {
        // Update service directory location
        updateServiceDirectoryUrl(serviceDirectoryUrl);

        // Get our domain
        if (componentID != "ServiceDirectory")
        {
            GravityDataProduct request("GetDomain");
            GravityDataProduct response("DomainResponse");
            ret = sendRequestToServiceDirectory(request, response);

            if (ret == GravityReturnCodes::SUCCESS)
            {
                char* p = (char*)calloc(response.getDataSize(), sizeof(char));
                response.getData(p, response.getDataSize());
                myDomain.assign(p, response.getDataSize());
                free(p);
            }
        }
        else
        {
            myDomain = getStringParam("Domain", "");
        }

        if (ret == GravityReturnCodes::SUCCESS)
        {
            if (componentID != "ServiceDirectory")
            {
                settingsPubEnabled = getBoolParam("GravitySettingsPublishEnabled", false);
                if (settingsPubEnabled)
                {
                    registerDataProductInternal(gravity::constants::GRAVITY_SETTINGS_DPID, GravityTransportTypes::TCP,
                                                false, false, false, true);
                }

                // Enable metrics (if configured)
                metricsEnabled = getBoolParam("GravityMetricsEnabled", false);
                if (metricsEnabled)
                {
                    // Register our metrics data product with the service directory
                    registerDataProductInternal(gravity::constants::METRICS_DATA_DPID, GravityTransportTypes::TCP,
                                                false, false, false, true);

                    // Command the GravityMetricsManager thread to start collecting metrics
                    sendStringMessage(metricsManagerSocket, "MetricsEnable", ZMQ_SNDMORE);

                    // Get collection parameters from ini file
                    // (default to 10 second sampling, publishing once per min)
                    int samplePeriod = getIntParam("GravityMetricsSamplePeriodSeconds", 10);
                    int samplesPerPublish = getIntParam("GravityMetricsSamplesPerPublish", 6);

                    // Send collection details to the GravityMetricsManager
                    sendIntMessage(metricsManagerSocket, samplePeriod, ZMQ_SNDMORE);
                    sendIntMessage(metricsManagerSocket, samplesPerPublish, ZMQ_SNDMORE);

                    // Finally, send our component id, ip address, and registration time (to be published with metrics)
                    sendStringMessage(metricsManagerSocket, componentID, ZMQ_SNDMORE);
                    sendStringMessage(metricsManagerSocket, getIP(), ZMQ_SNDMORE);
                    sendIntMessage(metricsManagerSocket, dataRegistrationTimeMap[gravity::constants::METRICS_DATA_DPID],
                                   ZMQ_DONTWAIT);
                }
            }

            if (componentID != "ConfigServer" && getBoolParam("NoConfigServer", false) != true)
            {
                parser->ParseConfigService(
                    *this);  //Although this is done last, this has the least priority.  We just need to do it last so we know where the service directory is located.
            }
            //parser->ParseCmdLine

            configureServiceManager();
            configureSubscriptionManager();

            // Auto start heartbeats if specified in INI
            double heartbeatPeriodSecs = getFloatParam("GravityHeartbeatPeriodSecs", -1);
            if (heartbeatPeriodSecs > 0)
            {
                logger->debug("Starting heartbeats ({} secs)", heartbeatPeriodSecs);
                int64_t micros = std::round(heartbeatPeriodSecs * 1e6);
                startHeartbeat(micros);
            }
        }
    }
    else
    {
        ret = GravityReturnCodes::FAILURE;
    }

    if (iniWarning)
    {
        logger->warn("Gravity.ini specifies both Domain and URL. Using URL.");
    }

    initLock.Unlock();

    return ret;
}
/*
 * Do a name lookup to convert a hostname to an IP address in ascii 
 * dotted-quad notation.
 */
static string toDottedQuad(string hostname)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    shared_ptr<spdlog::logger> logger = spdlog::get("GravityLogger");
    // others zero
    //logger->debug("Looking up quad for: {}\n", hostname);
    s = getaddrinfo(hostname.c_str(), NULL, &hints, &result);
    if (s != 0)
    {
        logger->warn("error in getaddrinfo: {}\n", gai_strerror(s));
        return hostname;
    }
    string quad = hostname;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        struct in_addr pp = (((struct sockaddr_in*)(rp->ai_addr))->sin_addr);
        //logger->debug("found addr quad for: {} : {}", hostname.c_str(), inet_ntoa(pp));
        quad.assign(inet_ntoa(pp));
        break;
    }

    freeaddrinfo(result); /* No longer needed */
    return quad;
}

void GravityNode::updateServiceDirectoryUrl(string serviceDirectoryUrl)
{
    // Extract URL component (transport, ip, and port)
    serviceDirectoryLock.Lock();
    size_t pos = serviceDirectoryUrl.find_first_of("://");
    if (pos != std::string::npos)
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
    if (serviceDirectoryNode.ipAddress == "" || serviceDirectoryNode.ipAddress == "*")
        serviceDirectoryNode.ipAddress = "localhost";

    if (serviceDirectoryNode.ipAddress != "localhost" && serviceDirectoryNode.ipAddress != "0.0.0.0")
    {
        //logger->debug("SD IP originally: {}", serviceDirectoryNode.ipAddress);
        serviceDirectoryNode.ipAddress = toDottedQuad(serviceDirectoryNode.ipAddress);
        //logger->debug("SD IP set to: {}", serviceDirectoryNode.ipAddress);
    }

    serviceDirectoryNode.port = gravity::StringToInt(serviceDirectoryUrl.substr(pos1 + 1), 5555);
    serviceDirectoryLock.Unlock();

    // Inform SubscriptionManager of Service Directory location
    subscriptionManagerSWL.lock.Lock();
    sendStringMessage(subscriptionManagerSWL.socket, "set_service_dir_url", ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, serviceDirectoryUrl, ZMQ_DONTWAIT);
    subscriptionManagerSWL.lock.Unlock();

    // Inform RequestManager of Service Directory location
    requestManagerSWL.lock.Lock();
    sendStringMessage(requestManagerSWL.socket, "set_service_dir_url", ZMQ_SNDMORE);
    sendStringMessage(requestManagerSWL.socket, serviceDirectoryUrl, ZMQ_DONTWAIT);
    requestManagerSWL.lock.Unlock();
}

void GravityNode::configureNodeDomainListener(std::string domain)
{
    sendStringMessage(domainListenerSWL.socket, "configure", ZMQ_SNDMORE);
    sendStringMessage(domainListenerSWL.socket, domain, ZMQ_SNDMORE);

    int port = getIntParam("ServiceDirectoryBroadcastPort", DEFAULT_BROADCAST_PORT);
    int broadcastTimeout = getIntParam("ServiceDirectoryBroadcastTimeout", DEFAULT_BROADCAST_TIMEOUT_SEC);

    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(port));
    memcpy(zmq_msg_data(&msg), &port, sizeof(port));
    zmq_sendmsg(domainListenerSWL.socket, &msg, ZMQ_SNDMORE);
    zmq_msg_close(&msg);

    zmq_msg_t msg2;
    zmq_msg_init_size(&msg2, sizeof(broadcastTimeout));
    memcpy(zmq_msg_data(&msg2), &broadcastTimeout, sizeof(broadcastTimeout));
    zmq_sendmsg(domainListenerSWL.socket, &msg2, ZMQ_SNDMORE);
    zmq_msg_close(&msg2);

    sendStringMessage(domainListenerSWL.socket, componentID, ZMQ_SNDMORE);

    zmq_msg_t msg3;
    zmq_msg_init_size(&msg3, sizeof(GravityNode*));
    GravityNode* tmp = this;
    memcpy(zmq_msg_data(&msg3), &tmp, sizeof(GravityNode*));
    zmq_sendmsg(domainListenerSWL.socket, &msg3, ZMQ_DONTWAIT);
    zmq_msg_close(&msg3);

    readStringMessage(domainListenerSWL.socket);
}

void GravityNode::configureServiceManager()
{
    sendStringMessage(serviceManagerConfigSWL.socket, "configure", ZMQ_SNDMORE);

    //send Domain
    sendStringMessage(serviceManagerConfigSWL.socket, myDomain, ZMQ_SNDMORE);

    //Send component ID
    sendStringMessage(serviceManagerConfigSWL.socket, componentID, ZMQ_DONTWAIT);
}

void GravityNode::configureSubscriptionManager()
{
    sendStringMessage(subscriptionManagerConfigSWL.socket, "configure", ZMQ_SNDMORE);

    //send Domain
    sendStringMessage(subscriptionManagerConfigSWL.socket, myDomain, ZMQ_SNDMORE);

    //Send component ID
    sendStringMessage(subscriptionManagerConfigSWL.socket, componentID, ZMQ_DONTWAIT);

    //Send ip address
    sendStringMessage(subscriptionManagerConfigSWL.socket, getIP(), ZMQ_DONTWAIT);
}

std::string GravityNode::getDomainUrl(int timeout)
{
    int waitTime = 0;

    zmq_pollitem_t pollItem;
    pollItem.socket = domainRecvSWL.socket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;
    std::string url = "";
    while (waitTime < timeout * 1000)
    {
        // Start polling socket(s), blocking while we wait
        int rc = zmq_poll(&pollItem, 1, 0);  // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            break;
        }

        // Process new subscription requests from the gravity node
        if (pollItem.revents & ZMQ_POLLIN)
        {
            std::string command = readStringMessage(domainRecvSWL.socket);
            if (command == "connect")
            {
                std::string domain = readStringMessage(domainRecvSWL.socket);
                url = readStringMessage(domainRecvSWL.socket);
                break;
            }
        }

        sleep(100);
        waitTime += 100;
    }
    return url;
}

void GravityNode::waitForExit()
{
    while (subscriptionManagerThread.joinable())
    {
        sleep(10000);
    }
}

GravityReturnCode GravityNode::sendRequestsToServiceProvider(string url, const GravityDataProduct& request,
                                                             GravityDataProduct& response, int timeout_in_milliseconds,
                                                             int retries)
{
    GravityReturnCode ret = GravityReturnCodes::NOT_INITIALIZED;

    int retriesLeft = retries;
    while (retriesLeft && ret != GravityReturnCodes::FAILURE && ret != GravityReturnCodes::SUCCESS)
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
    logger->trace("GravityNode::sendRequestToServiceProvider({},{},{},{})", url, request.getDataProductID(),
                  response.getDataProductID(), timeout_in_milliseconds);
    void* socket = zmq_socket(context, ZMQ_REQ);  // Socket to connect to service provider
    zmq_connect(socket, url.c_str());
    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    // Send message to service provider
    sendGravityDataProduct(socket, request, ZMQ_DONTWAIT);

    // Poll socket for reply with a timeout
    zmq_pollitem_t items[] = {{socket, 0, ZMQ_POLLIN, 0}};
    int rc = zmq_poll(items, 1, timeout_in_milliseconds);
    if (rc == -1)
    {
        if (errno == EINTR)
            ret = GravityReturnCodes::INTERRUPTED;
        else
            ret = GravityReturnCodes::FAILURE;
    }
    else if (rc == 0)
        ret = GravityReturnCodes::REQUEST_TIMEOUT;
    // Got a Response, now process it.  Process the response
    else if (items[0].revents & ZMQ_POLLIN)
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

    if (s_interrupted)
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
    ss << serviceDirectoryNode.transport << "://" << serviceDirectoryNode.ipAddress << ":" << serviceDirectoryNode.port;
    string serviceDirectoryURL = ss.str();
    logger->trace("About to make SD request at URL = {}", serviceDirectoryURL);

    GravityReturnCode ret =
        sendRequestsToServiceProvider(serviceDirectoryURL, request, response, NETWORK_TIMEOUT, NETWORK_RETRIES);
    serviceDirectoryLock.Unlock();
    return ret;
}

GravityReturnCode GravityNode::registerDataProduct(string dataProductID, GravityTransportType transportType)
{
    return registerDataProduct(dataProductID, transportType, defaultCacheLastSentDataprodut);
}

GravityReturnCode GravityNode::registerDataProduct(string dataProductID, GravityTransportType transportType,
                                                   bool cacheLastValue)
{
    return registerDataProductInternal(dataProductID, transportType, cacheLastValue, false, false, false);
}

GravityReturnCode GravityNode::subscribersExist(std::string dataProductID, bool& hasSubscribersOut)
{
    hasSubscribersOut = true;  // if error and the client doesn't check, safer to assume we have a subscriber
    GravityReturnCode ret = GravityReturnCode::FAILURE;
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    publishManagerRequestSWL.lock.Lock();
    sendStringMessage(publishManagerRequestSWL.socket, "subscribersExist", ZMQ_SNDMORE);
    sendStringMessage(publishManagerRequestSWL.socket, dataProductID, ZMQ_DONTWAIT);
    string result = readStringMessage(publishManagerRequestSWL.socket);
    publishManagerRequestSWL.lock.Unlock();
    if (result == "Y")
    {
        hasSubscribersOut = true;
        ret = GravityReturnCode::SUCCESS;
    }
    else if (result == "N")
    {
        hasSubscribersOut = false;
        ret = GravityReturnCode::SUCCESS;
    }
    else
    {
        logger->warn("Query for subscribers returns error : {}", result);
        if (result.rfind("Unknown data product", 0) == 0)
        {
            ret = GravityReturnCode::NOT_REGISTERED;
        }  // otherwise, leave as generic failure code and hasSubscribersOut=true
    }
    return ret;
}

GravityReturnCode GravityNode::registerDataProductInternal(std::string dataProductID,
                                                           GravityTransportType transportType, bool cacheLastValue,
                                                           bool isRelay, bool localOnly, bool allowReservedIDs)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }

    if ((componentID != "ServiceDirectory" && serviceDirectory_ReservedDataProductIDs.find(dataProductID) !=
                                                  serviceDirectory_ReservedDataProductIDs.end()) ||
        (allowReservedIDs == false &&
         gravityNode_ReservedDataProductIDs.find(dataProductID) != gravityNode_ReservedDataProductIDs.end()))
    {
        spdlog::warn("Rejecting attempt to register {}", dataProductID);
        return GravityReturnCodes::RESERVED_DATA_PRODUCT_ID;
    }

    std::string transportType_str;
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    // we can't allow multiple threads to make request calls to the pub manager at the same time
    // because the requests will step on each other.  Manage access to publishMap as well.
    publishManagerRequestSWL.lock.Lock();

    if (publishMap.count(dataProductID) > 0)
    {
        logger->warn("attempt to register duplicate data product ID: {}", dataProductID);
        publishManagerRequestSWL.lock.Unlock();
        return GravityReturnCodes::SUCCESS;
    }

    string endpoint;
    if (transportType == GravityTransportTypes::TCP)
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
    else if (transportType == GravityTransportTypes::INPROC)
    {
        transportType_str = "inproc";
        endpoint = dataProductID;
    }
    else if (transportType == GravityTransportTypes::PGM)
    {
        transportType_str = "pgm";
        endpoint = dataProductID;
    }
    else if (transportType == GravityTransportTypes::EPGM)
    {
        transportType_str = "epgm";
        endpoint = dataProductID;
    }

    // Registration timestamp - will serve as a unique identifier for this registered publication
    uint64_t timestamp = getCurrentTime();

    // Send publish details via the request socket.  This allows us to retrieve
    // register url in response so that we can register with the ServiceDirectory.
    sendStringMessage(publishManagerRequestSWL.socket, "register", ZMQ_SNDMORE);
    sendStringMessage(publishManagerRequestSWL.socket, dataProductID, ZMQ_SNDMORE);
    sendIntMessage(publishManagerRequestSWL.socket, cacheLastValue, ZMQ_SNDMORE);
    sendStringMessage(publishManagerRequestSWL.socket, transportType_str, ZMQ_SNDMORE);
    if (transportType == GravityTransportTypes::TCP)
    {
        int minPort = getIntParam("MinPort", MIN_PORT);
        int maxPort = getIntParam("MaxPort", MAX_PORT);
        sendIntMessage(publishManagerRequestSWL.socket, minPort, ZMQ_SNDMORE);
        sendIntMessage(publishManagerRequestSWL.socket, maxPort, ZMQ_SNDMORE);
    }
    sendStringMessage(publishManagerRequestSWL.socket, endpoint, ZMQ_DONTWAIT);

    string connectionURL = readStringMessage(publishManagerRequestSWL.socket);

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
            registration.set_is_relay(isRelay);
            if (localOnly)
            {
                registration.set_ip_address(getIP());
            }

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
        else
        {
            ret = GravityReturnCodes::NO_SERVICE_DIRECTORY;
        }
    }

    if (ret != GravityReturnCodes::SUCCESS)
    {
        logger->warn("Failed to register {} at url {} with error {}", dataProductID, connectionURL, getCodeString(ret));
        // if we didn't succesfully register with the SD, then unregister with the publish manager
        sendStringMessage(publishManagerRequestSWL.socket, "unregister", ZMQ_SNDMORE);
        sendStringMessage(publishManagerRequestSWL.socket, dataProductID, ZMQ_DONTWAIT);
        readStringMessage(publishManagerRequestSWL.socket);
    }
    else
    {
        logger->debug("Registered publisher at address: {}", connectionURL);
        publishMap[dataProductID] = connectionURL;
        urlInstanceMap[connectionURL] = timestamp;
        dataRegistrationTimeMap[dataProductID] = static_cast<uint32_t>(timestamp / 1e6);  // Maintained in epoch seconds
    }

    publishManagerRequestSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::unregisterDataProduct(string dataProductID)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
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
        uint32_t regTime = dataRegistrationTimeMap[dataProductID];
        dataRegistrationTimeMap.erase(dataProductID);

        if (!serviceDirectoryNode.ipAddress.empty())
        {
            ServiceDirectoryUnregistrationPB unregistration;
            unregistration.set_id(dataProductID);
            unregistration.set_url(url);
            unregistration.set_type(ServiceDirectoryUnregistrationPB::DATA);
            unregistration.set_registration_time(regTime);

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

GravityReturnCode GravityNode::ServiceDirectoryDataProductLookup(std::string dataProductID,
                                                                 vector<PublisherInfoPB>& urls, string& domain)
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
            for (int i = 0; i < pb.publishers_size(); i++) urls.push_back(pb.publishers(i));
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

GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret = subscribeInternal(dataProductID, subscriber, "", "", defaultReceiveLastSentDataproduct);
    subscriptionManagerSWL.lock.Unlock();
    return ret;
}
GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret = subscribeInternal(dataProductID, subscriber, filter, "", defaultReceiveLastSentDataproduct);
    subscriptionManagerSWL.lock.Unlock();
    return ret;
}
GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter,
                                         string domain)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret =
        subscribeInternal(dataProductID, subscriber, filter, domain, defaultReceiveLastSentDataproduct);
    subscriptionManagerSWL.lock.Unlock();
    return ret;
}

GravityReturnCode GravityNode::subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter,
                                         string domain, bool receiveLastCachedValue)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret = subscribeInternal(dataProductID, subscriber, filter, domain, receiveLastCachedValue);
    subscriptionManagerSWL.lock.Unlock();
    return ret;
}

GravityReturnCode GravityNode::subscribeInternal(string dataProductID, const GravitySubscriber& subscriber,
                                                 string filter, string domain, bool receiveLastCachedValue)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }

    if (domain.empty())
    {
        domain = myDomain;
    }

    vector<PublisherInfoPB> publisherInfoPBs;

    GravityReturnCode ret;
    ret = ServiceDirectoryDataProductLookup(dataProductID, publisherInfoPBs, domain);
    if (ret != GravityReturnCodes::SUCCESS)
    {
        return ret;
    }

    logger->trace("Subscribing to [{}] and receiving cached values: {}", dataProductID, receiveLastCachedValue);

    vector<PublisherInfoPB> registeredPublishersInfo;
    int tries = 5;
    while (registeredPublishersInfo.size() == 0 && tries-- > 0)
    {
        ret = ServiceDirectoryDataProductLookup(gravity::constants::REGISTERED_PUBLISHERS_DPID,
                                                registeredPublishersInfo, myDomain);
        if (ret != GravityReturnCodes::SUCCESS) return ret;
        if (registeredPublishersInfo.size() > 1)
            logger->warn("Found more than one ({}) Service Directory registered for publisher updates?",
                         registeredPublishersInfo.size());
        else if (registeredPublishersInfo.size() == 1)
            break;

        if (tries > 0) gravity::sleep(500);
    }

    if (registeredPublishersInfo.size() == 0 || !registeredPublishersInfo[0].has_url())
    {
        logger->error("Service Directory has not finished initialization ({} not available)",
                      gravity::constants::REGISTERED_PUBLISHERS_DPID);
        return GravityReturnCodes::NO_SERVICE_DIRECTORY;
    }

    // Send subscription details
    logger->trace("Sending subscription details to subscription manager");
    sendStringMessage(subscriptionManagerSWL.socket, "subscribe", ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);
    sendIntMessage(subscriptionManagerSWL.socket, receiveLastCachedValue, ZMQ_SNDMORE);
    sendUint32Message(subscriptionManagerSWL.socket, publisherInfoPBs.size(), ZMQ_SNDMORE);
    for (unsigned int i = 0; i < publisherInfoPBs.size(); i++)
    {
        sendProtobufMessage(subscriptionManagerSWL.socket, publisherInfoPBs[i], ZMQ_SNDMORE);
    }
    sendStringMessage(subscriptionManagerSWL.socket, filter, ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, domain, ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, registeredPublishersInfo[0].url(), ZMQ_SNDMORE);

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
    details.receiveLastCachedValue = receiveLastCachedValue;
    details.subscriber = &subscriber;
    subscriptionList.push_back(details);

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::unsubscribe(string dataProductID, const GravitySubscriber& subscriber, string filter,
                                           string domain)
{
    subscriptionManagerSWL.lock.Lock();
    GravityReturnCode ret = unsubscribeInternal(dataProductID, subscriber, filter, domain);
    subscriptionManagerSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::unsubscribeInternal(string dataProductID, const GravitySubscriber& subscriber,
                                                   string filter, string domain)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }

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
        if (iter->dataProductID == dataProductID && iter->domain == domain && iter->filter == filter &&
            iter->subscriber == &subscriber)
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

GravityReturnCode GravityNode::publish(const GravityDataProduct& dataProduct, std::string filterText,
                                       uint64_t timestamp)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }

    string dataProductID = dataProduct.getDataProductID();
    logger->trace("Publishing {}", dataProductID);

    // keep original info if this message has been relayed from somewhere else
    if (!dataProduct.isRelayedDataproduct())
    {
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
    }

    // Set registration time
    dataProduct.setRegistrationTime(dataRegistrationTimeMap[dataProductID]);

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
GravityReturnCode GravityNode::ServiceDirectoryReregister(string componentId, string url)
{
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;

    logger->warn("ServiceDirectory restart detected, attempting to re-register...");

    // Update service directory location
    updateServiceDirectoryUrl(url);

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
            logger->debug("Error re-registering publisher, retrying...");
            pubRet = sendRequestToServiceDirectory(request, response);
        }
        if (pubRet == GravityReturnCodes::SUCCESS)
        {
            logger->info("Successfully re-registered data product {}", iter->first);
        }
        else
        {
            logger->error("Error re-registering data product {}: {}", iter->first, getCodeString(pubRet));
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
            logger->debug("Error re-registering service, retrying...");
            servRet = sendRequestToServiceDirectory(request, response);
        }
        if (servRet == GravityReturnCodes::SUCCESS)
        {
            logger->info("Successfully re-registered service {}", iter->first);
        }
        else
        {
            logger->error("Error re-registering service {}: {}", iter->first, getCodeString(servRet));
            ret = servRet;
        }
    }
    serviceManagerSWL.lock.Unlock();

    subscriptionManagerSWL.lock.Lock();

    // Make a copy since subscriptionList will be updated as we re-subscribe to this list
    list<SubscriptionDetails> origList = subscriptionList;

    for (list<SubscriptionDetails>::const_iterator iter = origList.begin(); iter != origList.end(); ++iter)
    {
        GravityReturnCode subRet =
            subscribeInternal(iter->dataProductID, *iter->subscriber, iter->filter, iter->domain);
        int numTries = 3;
        while (subRet != GravityReturnCodes::SUCCESS && numTries-- > 0)
        {
            logger->debug("Error re-subscribing, retrying...");
            subRet = subscribeInternal(iter->dataProductID, *iter->subscriber, iter->filter, iter->domain);
        }
        logger->info("Successfully re-subscribed {}", iter->dataProductID);
    }
    subscriptionManagerSWL.lock.Unlock();

    if (ret == GravityReturnCodes::SUCCESS) logger->warn("Successfully re-registered with the ServiceDirectory");

    return ret;
}

GravityReturnCode GravityNode::ServiceDirectoryServiceLookup(std::string serviceID, std::string& url, string& domain,
                                                             uint32_t& regTime)
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
                regTime = pb.registration_time();
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
                                       const GravityRequestor& requestor, string requestID, int timeout_milliseconds,
                                       string domain)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    std::string url;
    uint32_t regTime;
    GravityReturnCode ret = ServiceDirectoryServiceLookup(serviceID, url, domain, regTime);
    if (ret != GravityReturnCodes::SUCCESS) return ret;
    return request(url, serviceID, dataProduct, requestor, regTime, requestID, timeout_milliseconds);
}

//Asynchronous Request with URL
GravityReturnCode GravityNode::request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
                                       const GravityRequestor& requestor, uint32_t regTime, string requestID,
                                       int timeout_milliseconds)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
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
    sendIntMessage(requestManagerSWL.socket, timeout_milliseconds, ZMQ_SNDMORE);
    sendUint32Message(requestManagerSWL.socket, regTime, ZMQ_SNDMORE);

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
std::shared_ptr<GravityDataProduct> GravityNode::request(string serviceID, const GravityDataProduct& request,
                                                         int timeout_milliseconds, string domain)
{
    if (!initialized)
    {
        return std::shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
    }
    //set Component ID
    request.setComponentId(componentID);

    //set Domain
    request.setDomain(myDomain);

    logger->trace("Synchronous request('{}','{}')", serviceID, request.getDataProductID());

    std::string connectionURL;
    uint32_t regTime;
    GravityReturnCode ret = ServiceDirectoryServiceLookup(serviceID, connectionURL, domain, regTime);
    if (ret != GravityReturnCodes::SUCCESS)
    {
        //special case for ConfigService. Since this happens frequently, log message not warning.
        if (serviceID == "ConfigService")
        {
            logger->info("Unable to find service {}: {}", serviceID, getCodeString(ret));
        }
        else
        {
            logger->warn("Unable to find service {}: {}", serviceID, getCodeString(ret));
        }
        return std::shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
    }

    uint64_t t1 = gravity::getCurrentTime();
    std::shared_ptr<GravityDataProduct> response(new GravityDataProduct(serviceID));
    logger->trace("Sending request to service provider @ {}", connectionURL);
    ret = sendRequestToServiceProvider(connectionURL, request, *response, timeout_milliseconds);
    if (ret != GravityReturnCodes::SUCCESS)
    {
        logger->warn("service request returned error: {}", getCodeString(ret));
        return std::shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
    }
    if (response->getRegistrationTime() != regTime)
    {
        logger->warn("Received service ({}) response from invalid service [{} != {}]", serviceID,
                     response->getRegistrationTime(), regTime);
        return std::shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
    }

    if (response->isFutureResponse())
    {
        logger->trace("Synchronous Request: received a future response");
        if (timeout_milliseconds > 0)
        {
            timeout_milliseconds -= (int)((gravity::getCurrentTime() - t1) / 1e3);
            if (timeout_milliseconds <= 0)
            {
                timeout_milliseconds = 1;
            }
        }
        logger->trace("Sending request to future response socket (url='{}', timeout={})",
                      response->getFutureSocketUrl(), timeout_milliseconds);
        ret = sendRequestToServiceProvider(response->getFutureSocketUrl(), request, *response, timeout_milliseconds);
        if (ret != GravityReturnCodes::SUCCESS)
        {
            logger->warn("service request returned error: {}", getCodeString(ret));
            return std::shared_ptr<GravityDataProduct>((GravityDataProduct*)NULL);
        }
        logger->trace("Received future response's response");
    }

    return response;
}

GravityReturnCode GravityNode::registerService(string serviceID, GravityTransportType transportType,
                                               const GravityServiceProvider& server)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    string transportType_str;

    // manage access to service manager socket as well as serviceMap.
    serviceManagerSWL.lock.Lock();
    if (serviceMap.count(serviceID) > 0)
    {
        logger->warn("attempt to register duplicate service ID: {}", serviceID);
        serviceManagerSWL.lock.Unlock();
        return GravityReturnCodes::SUCCESS;
    }

    // Build the connection string
    string endpoint;
    if (transportType == GravityTransportTypes::TCP)
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
    else if (transportType == GravityTransportTypes::INPROC)
    {
        transportType_str = "inproc";
        endpoint = serviceID;
    }
    else if (transportType == GravityTransportTypes::PGM)
    {
        transportType_str = "pgm";
        endpoint = serviceID;
    }
    else if (transportType == GravityTransportTypes::EPGM)
    {
        transportType_str = "epgm";
        endpoint = serviceID;
    }

    // Send subscription details
    uint64_t timestamp = getCurrentTime();

    sendStringMessage(serviceManagerSWL.socket, "register", ZMQ_SNDMORE);
    sendStringMessage(serviceManagerSWL.socket, serviceID, ZMQ_SNDMORE);
    sendStringMessage(serviceManagerSWL.socket, transportType_str, ZMQ_SNDMORE);

    if (transportType == GravityTransportTypes::TCP)
    {
        int minPort = getIntParam("MinPort", MIN_PORT);
        int maxPort = getIntParam("MaxPort", MAX_PORT);
        sendIntMessage(serviceManagerSWL.socket, minPort, ZMQ_SNDMORE);
        sendIntMessage(serviceManagerSWL.socket, maxPort, ZMQ_SNDMORE);
    }
    sendStringMessage(serviceManagerSWL.socket, endpoint, ZMQ_SNDMORE);
    sendUint32Message(serviceManagerSWL.socket, static_cast<uint32_t>(timestamp / 1e6), ZMQ_SNDMORE);

    // Include the server
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(&server));
    void* v = (void*)&server;
    memcpy(zmq_msg_data(&msg), &v, sizeof(&server));
    zmq_sendmsg(serviceManagerSWL.socket, &msg, ZMQ_DONTWAIT);
    zmq_msg_close(&msg);

    string connectionURL = readStringMessage(serviceManagerSWL.socket);

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
        logger->debug("Registered service at address: {} ({})", connectionURL, timestamp);
        serviceMap[serviceID] = connectionURL;
        urlInstanceMap[connectionURL] = timestamp;
    }
    serviceManagerSWL.lock.Unlock();
    return ret;
}

GravityReturnCode GravityNode::unregisterService(string serviceID)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
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
        if (urlInstanceMap.count(url) == 0)
        {
            return GravityReturnCodes::REGISTRATION_CONFLICT;
        }

        serviceMap.erase(serviceID);

        string status = readStringMessage(serviceManagerSWL.socket);

        if (!serviceDirectoryNode.ipAddress.empty())
        {
            ServiceDirectoryUnregistrationPB unregistration;
            unregistration.set_id(serviceID);
            unregistration.set_url(url);
            unregistration.set_type(ServiceDirectoryUnregistrationPB::SERVICE);
            unregistration.set_registration_time(static_cast<uint32_t>(urlInstanceMap[url] / 1e6));

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
        urlInstanceMap.erase(url);
    }
    serviceManagerSWL.lock.Unlock();

    return ret;
}

GravityReturnCode GravityNode::stopHeartbeat()
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    GravityReturnCode ret = gravity::GravityReturnCodes::SUCCESS;
    if (heartbeatStarted)
    {
        heartbeatStarted = false;

        // Send signal to heartbeat thread to stop
        Heartbeat::setHeartbeatRunning(false);

        // Unregister heartbeat with Service Directory
        std::string heartbeatName = componentID + "_GravityHeartbeat";
        ret = unregisterDataProduct(heartbeatName);
    }

    return ret;
}

GravityReturnCode GravityNode::startHeartbeat(int64_t interval_in_microseconds)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    if (interval_in_microseconds < 0) return gravity::GravityReturnCodes::FAILURE;

    if (heartbeatStarted) return gravity::GravityReturnCodes::FAILURE;  //We shouldn't be able to start this guy twice

    std::string heartbeatName;
    //Gravity Heartbeats named by component ID
    heartbeatName = componentID + "_GravityHeartbeat";

    this->registerDataProductInternal(heartbeatName, GravityTransportTypes::TCP, false, false, false, true);

    HBParams* params = new HBParams();  //(freed by thread)
    params->zmq_context = context;
    params->interval_in_microseconds = interval_in_microseconds;
    params->componentID = heartbeatName;
    params->minPort = getIntParam("MinPort", MIN_PORT);
    params->maxPort = getIntParam("MaxPort", MAX_PORT);
    params->endpoint = getIP();
    params->registrationTime = dataRegistrationTimeMap[heartbeatName];

    std::thread heartbeatThread(Heartbeat, (void*)params);
    heartbeatThread.detach();
    heartbeatStarted = true;

    return gravity::GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::registerHeartbeatListener(string componentID, int64_t timebetweenMessages,
                                                         const GravityHeartbeatListener& listener, string domain)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    void* HeartbeatListener(void*);  //Forward declaration.
    static class Heartbeat hbSub;
    GravityReturnCode ret = GravityReturnCodes::SUCCESS;
    if (hbSocket == NULL)
    {
        //Initialize Heartbeat thread.
        hbSocket = zmq_socket(context, ZMQ_REQ);
        zmq_bind(hbSocket, "inproc://heartbeat_listener");
        HBListenerContext* thread_context = new HBListenerContext();
        thread_context->zmq_context = this->context;
        std::thread heartbeatListenerThread(Heartbeat::HeartbeatListenerThrFunc, thread_context);
        heartbeatListenerThread.detach();
    }

    std::string heartbeatName;

    //Gravity Heartbeats named by component ID
    heartbeatName = componentID + "_GravityHeartbeat";

    ret = this->subscribe(heartbeatName, hbSub, "", domain);

    if (ret == GravityReturnCodes::SUCCESS)
    {
        //Send the DataproductID
        sendStringMessage(hbSocket, "register", ZMQ_SNDMORE);
        sendStringMessage(hbSocket, heartbeatName, ZMQ_SNDMORE);

        //Send the address of the listener
        zmq_msg_t msg1;
        zmq_msg_init_size(&msg1, sizeof(GravityHeartbeatListener*));
        intptr_t p = (intptr_t)&listener;
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
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    static class Heartbeat hbSub;

    std::string heartbeatName;
    heartbeatName = componentID + "_GravityHeartbeat";

    this->unsubscribe(heartbeatName, hbSub);

    //Send the DataproductID
    sendStringMessage(hbSocket, "unregister", ZMQ_SNDMORE);
    sendStringMessage(hbSocket, heartbeatName, ZMQ_DONTWAIT);

    // Read the ACK
    readStringMessage(hbSocket);

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::registerRelay(string dataProductID, const GravitySubscriber& subscriber, bool localOnly,
                                             GravityTransportType transportType)
{
    return registerRelay(dataProductID, subscriber, localOnly, transportType, false);
}

GravityReturnCode GravityNode::registerRelay(string dataProductID, const GravitySubscriber& subscriber, bool localOnly,
                                             GravityTransportType transportType, bool cacheLastValue)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    if (cacheLastValue)
    {
        logger->warn(
            "Using a Relay with cacheLastValue=true is atypical and may result in duplicate messages received by "
            "subscribers during the relay start/stop transition");
    }
    GravityReturnCode ret =
        registerDataProductInternal(dataProductID, transportType, cacheLastValue, true, localOnly, false);
    if (ret != GravityReturnCodes::SUCCESS) return ret;
    return subscribe(dataProductID, subscriber, "", "", false);
}

GravityReturnCode GravityNode::unregisterRelay(std::string dataProductID, const GravitySubscriber& subscriber)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    GravityReturnCode ret = unregisterDataProduct(dataProductID);
    if (ret != GravityReturnCodes::SUCCESS)
    {
        // try to unsubscribe in any case
        unsubscribe(dataProductID, subscriber);
        return ret;  // but return unregister error
    }
    return unsubscribe(dataProductID, subscriber);
}

string GravityNode::getDomain() { return myDomain; }

std::shared_ptr<FutureResponse> GravityNode::createFutureResponse()
{
    if (!initialized)
    {
        return std::shared_ptr<FutureResponse>((FutureResponse*)NULL);
    }
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
        logger->critical("Could not find available port for FutureResponse");
        return std::shared_ptr<FutureResponse>((FutureResponse*)NULL);
    }

    requestManagerRepSWL.lock.Unlock();

    std::shared_ptr<FutureResponse> futureResponse(new FutureResponse(url));
    return futureResponse;
}

GravityReturnCode GravityNode::sendFutureResponse(const FutureResponse& futureResponse)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    requestManagerRepSWL.lock.Lock();

    sendStringMessage(requestManagerRepSWL.socket, "sendFutureResponse", ZMQ_SNDMORE);
    sendStringMessage(requestManagerRepSWL.socket, futureResponse.getUrl(), ZMQ_SNDMORE);

    // Send the response object
    int size = futureResponse.getDataSize();
    char* bytes = new char[size];
    futureResponse.getData(bytes, size);
    GravityDataProduct response(bytes, size);
    response.setComponentId(componentID);
    response.setDomain(myDomain);
    sendGravityDataProduct(requestManagerRepSWL.socket, response, ZMQ_DONTWAIT);
    delete[] bytes;

    // Get results back from GravityRequestManager
    int ret = readIntMessage(requestManagerRepSWL.socket);

    requestManagerRepSWL.lock.Unlock();

    return static_cast<GravityReturnCode>(ret);
}

GravityReturnCode GravityNode::setSubscriptionTimeoutMonitor(string dataProductID,
                                                             const GravitySubscriptionMonitor& monitor,
                                                             int milliSecondTimeout, string filter, string domain)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    if (domain.empty())
    {
        domain = myDomain;
    }

    if (milliSecondTimeout <= 0)
    {
        return GravityReturnCodes::INVALID_PARAMETER;
    }

    subscriptionManagerSWL.lock.Lock();

    sendStringMessage(subscriptionManagerSWL.socket, "set_monitor", ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);

    //Send the address of the monitor
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(GravitySubscriptionMonitor*));
    intptr_t p = (intptr_t)&monitor;
    memcpy(zmq_msg_data(&msg), &p, sizeof(GravitySubscriptionMonitor*));
    zmq_sendmsg(subscriptionManagerSWL.socket, &msg, ZMQ_SNDMORE);
    zmq_msg_close(&msg);

    //Send the Max time between messages
    sendIntMessage(subscriptionManagerSWL.socket, milliSecondTimeout, ZMQ_SNDMORE);

    sendStringMessage(subscriptionManagerSWL.socket, filter, ZMQ_SNDMORE);

    sendStringMessage(subscriptionManagerSWL.socket, domain, ZMQ_DONTWAIT);

    //int ret = readIntMessage(subscriptionManagerSWL.socket);

    subscriptionManagerSWL.lock.Unlock();

    return GravityReturnCodes::SUCCESS;
}

GravityReturnCode GravityNode::clearSubscriptionTimeoutMonitor(std::string dataProductID,
                                                               const GravitySubscriptionMonitor& monitor, string filter,
                                                               string domain)
{
    if (!initialized)
    {
        return GravityReturnCodes::NOT_INITIALIZED;
    }
    subscriptionManagerSWL.lock.Lock();

    sendStringMessage(subscriptionManagerSWL.socket, "clear_monitor", ZMQ_SNDMORE);
    sendStringMessage(subscriptionManagerSWL.socket, dataProductID, ZMQ_SNDMORE);

    //Send the address of the monitor
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(GravitySubscriptionMonitor*));
    intptr_t p = (intptr_t)&monitor;
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
    if (!initialized)
    {
        return "";
    }

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
    configParamPB.set_key(key);

    if (parser == NULL || !(parser->hasKey(key)))
    {
        configParamPB.set_value(default_value);
        configParamPB.set_is_default(true);
    }
    else
    {
        configParamPB.set_value(parser->getString(key, default_value));
        configParamPB.set_is_default(false);
    }

    if (settingsPubEnabled && initialized &&
        !(publishedSettings.count(configParamPB.key()) == 1 &&
          publishedSettings.at(configParamPB.key()) == configParamPB.value()))
    {
        settingsGDP.setData(configParamPB);
        publish(settingsGDP, componentID);
        publishedSettings[configParamPB.key()] = configParamPB.value();
    }

    return configParamPB.value();
}

int GravityNode::getIntParam(std::string key, int default_value)
{
    std::string s = getStringParam(key, std::to_string(default_value));
    return StringToInt(s, default_value);
}

double GravityNode::getFloatParam(std::string key, double default_value)
{
    std::string s = getStringParam(key, std::to_string(default_value));
    return StringToDouble(s, default_value);
}

bool GravityNode::getBoolParam(std::string key, bool default_value)
{
    std::string val = StringToLowerCase(getStringParam(key, default_value ? "true" : "false"));

    if (val == "true" || val == "t" || val == "yes" || val == "y")
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string GravityNode::getComponentID() { return componentID; }

static std::map<GravityReturnCode, std::string> code_strings = {
    {GravityReturnCodes::SUCCESS, "SUCCESS"},
    {GravityReturnCodes::FAILURE, "FAILURE"},
    {GravityReturnCodes::NO_SERVICE_DIRECTORY, "NO_SERVICE_DIRECTORY"},
    {GravityReturnCodes::REQUEST_TIMEOUT, "REQUEST_TIMEOUT"},
    {GravityReturnCodes::DUPLICATE, "DUPLICATE"},
    {GravityReturnCodes::REGISTRATION_CONFLICT, "REGISTRATION_CONFLICT"},
    {GravityReturnCodes::NOT_REGISTERED, "NOT_REGISTERED"},
    {GravityReturnCodes::NO_SUCH_SERVICE, "NO_SUCH_SERVICE"},
    {GravityReturnCodes::LINK_ERROR, "LINK_ERROR"},
    {GravityReturnCodes::INTERRUPTED, "INTERRUPTED"},
    {GravityReturnCodes::NO_SERVICE_PROVIDER, "NO_SERVICE_PROVIDER"},
    {GravityReturnCodes::NO_PORTS_AVAILABLE, "NO_PORTS_AVAILABLE"},
    {GravityReturnCodes::INVALID_PARAMETER, "INVALID_PARAMETER"},
    {GravityReturnCodes::NOT_INITIALIZED, "NOT_INITIALIZED"}};

string GravityNode::getCodeString(GravityReturnCode code)
{
    std::string s;
    if (code_strings.count(code) == 0)
    {
        ostringstream convert;
        convert << code;
        s = convert.str();
    }
    else
    {
        s = code_strings[code];
    }
    return s;
}

std::shared_ptr<spdlog::logger> GravityNode::getGravityLogger() { return spdlog::get("GravityLogger"); }

} /* namespace gravity */
