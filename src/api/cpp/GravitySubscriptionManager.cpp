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
 * GravitySubscriptionManager.cpp
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#include "GravitySubscriptionManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include "GravityMetricsUtil.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"

#include <memory>
#include <algorithm>

namespace gravity
{

using namespace std;

bool sortCacheValues(const std::shared_ptr<GravityDataProduct>& i, const std::shared_ptr<GravityDataProduct>& j)
{
    return i->getGravityTimestamp() < j->getGravityTimestamp();
}

GravitySubscriptionManager::GravitySubscriptionManager(void* context)
{
    // This is the zmq context that is shared with the GravityNode. Must use
    // a shared context to establish an inproc socket.
    this->context = context;

    // Default to no metrics
    metricsEnabled = false;

    // Default high water mark
    subscribeHWM = 1000;

    // Get logger
    logger = spdlog::get("GravityLogger");
}

GravitySubscriptionManager::~GravitySubscriptionManager() {}

void GravitySubscriptionManager::start()
{
    vector<pair<std::shared_ptr<SubscriptionDetails>, void*> > deleteList;

    // Set up the inproc socket to subscribe to subscribe and unsubscribe messages from
    // the GravityNode
    gravityNodeSocket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(gravityNodeSocket, "inproc://gravity_subscription_manager");
    zmq_setsockopt(gravityNodeSocket, ZMQ_SUBSCRIBE, NULL, 0);

    // Setup socket to reponsd to metrics requests
    gravityMetricsSocket = zmq_socket(context, ZMQ_REP);
    zmq_bind(gravityMetricsSocket, GRAVITY_SUB_METRICS_REQ);

    // Poll the gravity node
    zmq_pollitem_t pollItem;
    pollItem.socket = gravityNodeSocket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;
    pollItems.push_back(pollItem);

    // Poll the metrics request socket
    zmq_pollitem_t metricsRequestPollItem;
    metricsRequestPollItem.socket = gravityMetricsSocket;
    metricsRequestPollItem.events = ZMQ_POLLIN;
    metricsRequestPollItem.fd = 0;
    metricsRequestPollItem.revents = 0;
    pollItems.push_back(metricsRequestPollItem);

    void* configureSocket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(configureSocket, "inproc://gravity_subscription_manager_configure");
    zmq_setsockopt(configureSocket, ZMQ_SUBSCRIBE, NULL, 0);

    // Always have at least the gravity node to poll
    zmq_pollitem_t configItem;
    configItem.socket = configureSocket;
    configItem.events = ZMQ_POLLIN;
    configItem.fd = 0;
    configItem.revents = 0;

    ready();

    //receive Gravity node parameters
    bool configured = false;
    while (!configured)
    {
        // Start polling socket(s), blocking while we wait
        int rc = zmq_poll(&configItem, 1, -1);  // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            if (errno == EINTR) continue;
            // Error
            break;
        }

        // Process new subscription requests from the gravity node
        if (configItem.revents & ZMQ_POLLIN)
        {
            string command = readStringMessage(configureSocket);
            if (command == "configure")
            {
                //receive Gravity node details
                domain = readStringMessage(configureSocket);
                componentID = readStringMessage(configureSocket);
                ipAddress = readStringMessage(configureSocket);

                configured = true;
            }
        }
    }

    zmq_close(configureSocket);

    // Process forever...
    while (true)
    {
        calculateTimeout();
        int rc = zmq_poll(&pollItems[0], pollItems.size(), pollTimeout);  // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            if (errno == EINTR)
            {
                continue;
            }
            // Error
            logger->debug("zmq_poll error, exiting (errno = {})", errno);
            break;
        }

        // If timeout occured
        if (rc == 0)
        {
            if (currTimeoutMonitor != NULL)
            {
                //calculate time since last subscription
                uint64_t currTime = getCurrentTime() / 1000;
                int timeSinceLast =
                    (currTimeoutMonitor->lastReceived) != -1l ? (int)(currTime - currTimeoutMonitor->lastReceived) : -1;
                (currTimeoutMonitor->monitor)
                    ->subscriptionTimeout(currMonitorDetails->dataProductID, timeSinceLast, currMonitorDetails->filter,
                                          currMonitorDetails->domain);
                //reset timeout for the current monitor
                currTimeoutMonitor->endTime = currTimeoutMonitor->endTime + currTimeoutMonitor->timeout;
                logger->trace("Subscription Timeout ({})", currMonitorDetails->dataProductID);
            }
            continue;
        }

        // Process new subscription requests from the gravity node
        if (pollItems[0].revents & ZMQ_POLLIN)
        {
            // Get new GravityNode request
            string command = readStringMessage(gravityNodeSocket);

            logger->trace("Received command [{}]", command);

            // message from gravity node should be either a subscribe or unsubscribe request
            if (command == "subscribe")
            {
                logger->trace("About to add subscription");
                addSubscription();
            }
            else if (command == "unsubscribe")
            {
                removeSubscription();
            }
            else if (command == "kill")
            {
                break;
            }
            else if (command == "set_hwm")
            {
                setHWM();
            }
            else if (command == "set_monitor")
            {
                setTimeoutMonitor();
            }
            else if (command == "clear_monitor")
            {
                clearTimeoutMonitor();
            }
            else if (command == "set_service_dir_url")
            {
                serviceDirectoryUrl = readStringMessage(gravityNodeSocket);
            }
            else
            {
                logger->warn("GravitySubscriptionManager received unknown command '{}' from GravityNode", command);
            }
        }

        if (pollItems[1].revents & ZMQ_POLLIN)
        {
            // Received a command from the metrics control
            void* socket = pollItems[1].socket;
            string command = readStringMessage(socket);

            if (command == "MetricsEnable")
            {
                // Clear any metrics data
                metricsData.reset();

                // Enable metrics
                metricsEnabled = true;

                // Acknowledge message
                sendStringMessage(socket, "ACK", ZMQ_DONTWAIT);
            }
            else if (command == "MetricsDisable")
            {
                // Disable metrics
                metricsEnabled = false;

                // Acknowledge message
                sendStringMessage(socket, "ACK", ZMQ_DONTWAIT);
            }
            else if (command == "GetMetrics")
            {
                // The GravityMetricsManager has request our metrics data

                // Mark the collection as completed
                metricsData.done();

                // Respond with metrics
                metricsData.sendAsMessage(socket);

                // Clear metrics data
                metricsData.reset();
            }
        }

        // Check for subscription updates
        for (unsigned int index = 2; index < pollItems.size(); index++)
        {
            string url = "<PUBLISHER UPDATES>";
            if (subscriptionSocketMap.find(pollItems[index].socket) != subscriptionSocketMap.end())
            {
                url = subscriptionSocketMap[pollItems[index].socket]->socketToUrlMap[pollItems[index].socket];
            }
            logger->trace("Checking poll item: index = {}, size = {}, url={}", index, pollItems.size(), url);
            if (pollItems[index].revents & ZMQ_POLLIN)
            {
                // if it's a regular subscription poll item
                if (subscriptionSocketMap.count(pollItems[index].socket) > 0)
                {
                    // Deliver to subscriber(s)
                    std::shared_ptr<SubscriptionDetails> subDetails = subscriptionSocketMap[pollItems[index].socket];

                    vector<std::shared_ptr<GravityDataProduct> > dataProducts;
                    while (true)
                    {
                        string filterText;
                        std::shared_ptr<GravityDataProduct> dataProduct;
                        if (readSubscription(pollItems[index].socket, filterText, dataProduct) < 0) break;

                        // Verify publisher
                        if (socketVerificationMap[pollItems[index].socket] != dataProduct->getRegistrationTime())
                        {
                            // Published data does not match publisher
                            logger->critical(
                                "Received data product ({}) from publisher different from registered with "
                                "ServiceDirectory [{} != {}]",
                                dataProduct->getDataProductID(), socketVerificationMap[pollItems[index].socket],
                                dataProduct->getRegistrationTime());

                            // Notify Service Directory of stale entry
                            notifyServiceDirectoryOfStaleEntry(subDetails->dataProductID, subDetails->domain, url,
                                                               socketVerificationMap[pollItems[index].socket]);

                            // Unsubscribe
                            //unsubscribeFromPollItem(pollItems[index], filterText);

                            // Add this socket to the to be deleted list
                            deleteList.push_back(std::make_pair(subDetails, pollItems[index].socket));

                            break;
                        }

                        std::shared_ptr<GravityDataProduct> lastCachedValue =
                            lastCachedValueMap[pollItems[index].socket];

                        // if it's been relayed, it will come in on a different socket than the original.  Look at all cached values
                        // to try to filter out duplicates.
                        // This is just to handle the transition when the Relay is first inserted - after that, normal subscribers
                        // will only be subscribed to the relay (if one exists).
                        bool cachedRelay = false;
                        if (!lastCachedValue && dataProduct->isRelayedDataproduct())
                        {
                            for (map<void*, std::shared_ptr<GravityDataProduct> >::const_iterator mapIter =
                                     lastCachedValueMap.begin();
                                 mapIter != lastCachedValueMap.end(); mapIter++)
                            {
                                if (mapIter->second &&
                                    mapIter->second->getGravityTimestamp() == dataProduct->getGravityTimestamp() &&
                                    mapIter->second->getComponentId() == dataProduct->getComponentId() &&
                                    *(mapIter->second) == *dataProduct)  // only compares product id and data
                                {
                                    cachedRelay = true;
                                    break;
                                }
                            }
                        }

                        // This may be a resend of previous value if a new subscriber was added, so make sure this is new data
                        if (!cachedRelay &&
                            (!lastCachedValue ||
                             lastCachedValue->getGravityTimestamp() < dataProduct->getGravityTimestamp() ||
                             // or if timestamps are the same, but GDP's are different
                             (lastCachedValue->getGravityTimestamp() == dataProduct->getGravityTimestamp() &&
                              !(*lastCachedValue == *dataProduct))))
                        {
                            // Grab current time now for stamping received_timestamp on received data products
                            dataProduct->setReceivedTimestamp(getCurrentTime());

                            if (dataProduct->isCachedDataproduct() && subDetails->receiveCachedDataProducts == false)
                            {
                                // if it's cached and we're not receiving cached, do nothing
                                logger->trace("Ignoring cached data product");
                            }
                            else
                            {
                                // Add data product to vector to be provided to the subscriber
                                logger->trace(dataProduct->isCachedDataproduct() ? "Accepting cached data product"
                                                                                 : "Accepting new data product");
                                dataProducts.push_back(dataProduct);
                            }
                            // Save most recent value so we can provide it to new subscribers, and to perform check above.
                            lastCachedValueMap[pollItems[index].socket] = dataProduct;
                        }
                    }

                    // Loop through all subscribers and deliver the messages
                    if (dataProducts.size() != 0)
                    {
                        logger->trace("received {} gdp's, about to send to {} subscribers", dataProducts.size(),
                                      subDetails->subscribers.size());

                        for (set<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin();
                             iter != subDetails->subscribers.end(); iter++)
                        {
                            (*iter)->subscriptionFilled(dataProducts);
                        }
                        uint64_t currTime = getCurrentTime() / 1000;
                        for (set<std::shared_ptr<TimeoutMonitor> >::iterator iter = subDetails->monitors.begin();
                             iter != subDetails->monitors.end(); iter++)
                        {
                            (*iter)->lastReceived = (int64_t)currTime;
                            (*iter)->endTime = currTime + (*iter)->timeout;
                        }

                        if (metricsEnabled)
                        {
                            collectMetrics(dataProducts);
                        }
                    }
                }
                else  // it's a publisher update list from the SD
                {
                    std::shared_ptr<GravityDataProduct> dataProduct;
                    string dataProductID;
                    readSubscription(pollItems[index].socket, dataProductID, dataProduct);
                    if (dataProductID.empty())
                    {
                        logger->warn("Received an apparently empty update list from ServiceDirectory");
                        continue;
                    }
                    ComponentDataLookupResponsePB update;
                    dataProduct->populateMessage(update);
                    string domain = update.domain_id();
                    logger->trace("Found update to publishers list for data product {} (domain:{})", dataProductID,
                                  domain);

                    // Create the domain/data key for tracking subscriptions
                    DomainDataKey key(domain, dataProductID);
                    logger->trace("subscriptionMap.count(key) = {}", subscriptionMap.count(key));

                    list<PublisherInfoPB> allPublishers, trimmedPublishers;
                    for (int i = 0; i < update.publishers_size(); i++)
                    {
                        allPublishers.push_back(update.publishers(i));
                    }
                    trimPublishers(allPublishers, trimmedPublishers);

                    if (subscriptionMap.count(key) != 0)
                    {
                        // Loop over our existing subscriptions (filters) for domain/data of the updated publisher list
                        for (map<string, std::shared_ptr<SubscriptionDetails> >::iterator iter =
                                 subscriptionMap[key].begin();
                             iter != subscriptionMap[key].end(); iter++)
                        {
                            // Details of the existing subscription
                            string filter = iter->first;
                            std::shared_ptr<SubscriptionDetails> subDetails = iter->second;

                            // Loop over publishers list provided by SD
                            for (list<PublisherInfoPB>::const_iterator trimmedIter = trimmedPublishers.begin();
                                 trimmedIter != trimmedPublishers.end(); trimmedIter++)
                            {
                                // If we don't already have this publisher url, add it OR if it is an updated publisher url based on a new registration timestamp
                                if (subDetails->pollItemMap.count(trimmedIter->url()) == 0 ||
                                    socketVerificationMap[subDetails->pollItemMap[trimmedIter->url()].socket] !=
                                        trimmedIter->registration_time())
                                {
                                    if (socketVerificationMap[subDetails->pollItemMap[trimmedIter->url()].socket] !=
                                        trimmedIter->registration_time())
                                    {
                                        logger->trace("Updated url: {}", trimmedIter->url());
                                        // This is an updated publisher location. Remove the old one.
                                        deleteList.push_back(std::make_pair(
                                            subDetails, subDetails->pollItemMap[trimmedIter->url()].socket));
                                    }
                                    else
                                    {
                                        logger->trace("New url: {}", trimmedIter->url());
                                    }

                                    zmq_pollitem_t pollItem;
                                    void* subSocket = setupSubscription(trimmedIter->url(), filter, pollItem);

                                    // Track by socket for quick lookup as data arrives
                                    subscriptionSocketMap[subSocket] = subDetails;

                                    // Add url & poll item to subscription details
                                    subDetails->pollItemMap[trimmedIter->url()] = pollItem;

                                    // Add lookup from socket to url
                                    subDetails->socketToUrlMap[subSocket] = trimmedIter->url();
                                }
                                else
                                {
                                    logger->trace("Skipping. Already have this publisher url: {}", trimmedIter->url());
                                }

                                // Insert/Update map to manage publisher verification
                                socketVerificationMap[iter->second->pollItemMap[trimmedIter->url()].socket] =
                                    trimmedIter->registration_time();
                            }

                            // loop through the existing urls/sockets to see if any have disappeared.
                            map<string, zmq_pollitem_t>::iterator socketIter = subDetails->pollItemMap.begin();
                            while (socketIter != subDetails->pollItemMap.end())
                            {
                                bool found = false;
                                // there doesn't seem to be a good way to check containment in a protobuf set...
                                for (list<PublisherInfoPB>::const_iterator trimmedIter = trimmedPublishers.begin();
                                     trimmedIter != trimmedPublishers.end(); trimmedIter++)
                                {
                                    if (socketIter->first == trimmedIter->url())
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found)
                                {
                                    logger->debug("url {} is gone, adding to delete list", socketIter->first);
                                    deleteList.push_back(std::make_pair(iter->second, socketIter->second.socket));
                                }
                                ++socketIter;
                            }
                        }
                    }
                }
            }
        }

        for (vector<pair<std::shared_ptr<SubscriptionDetails>, void*> >::iterator iter = deleteList.begin();
             iter != deleteList.end(); iter++)
        {
            std::shared_ptr<SubscriptionDetails> subDetails = iter->first;
            void* socket = iter->second;
            string url = subDetails->socketToUrlMap[socket];
            subscriptionSocketMap.erase(socket);
            socketVerificationMap.erase(socket);
            lastCachedValueMap.erase(socket);

            // Unsubscribe
            logger->trace("Unsubscribing: {}:{}:{} @ {}", subDetails->domain, subDetails->dataProductID,
                          subDetails->filter, url);
            zmq_setsockopt(socket, ZMQ_UNSUBSCRIBE, subDetails->filter.c_str(), subDetails->filter.length());

            // Close the socket
            zmq_close(socket);

            // If the socket for this url hasn't been updated, remove it
            if (subDetails->pollItemMap[url].socket == socket)
            {
                subDetails->pollItemMap.erase(url);
            }
            subDetails->socketToUrlMap.erase(socket);

            for (vector<zmq_pollitem_t>::iterator pollIter = pollItems.begin(); pollIter != pollItems.end(); ++pollIter)
            {
                if (socket == pollIter->socket)
                {
                    pollItems.erase(pollIter);
                    logger->debug("delete socket from pollitems, size is now {}", pollItems.size());
                    break;
                }
            }

            // Check if we're out of subscriptions for this domain/data/filter
            /*
			if (subDetails->pollItemMap.empty())
			{
				// Clean up update publisher subscription
				logger->debug("Unsubscribing from publisher updates");
				zmq_setsockopt(subDetails->publisherUpdatePollItem.socket, ZMQ_UNSUBSCRIBE, subDetails->dataProductID.c_str(),
								subDetails->dataProductID.length());
				zmq_close(subDetails->publisherUpdatePollItem.socket);

				for (vector<zmq_pollitem_t>::iterator pollIter = pollItems.begin(); pollIter != pollItems.end(); ++pollIter)
				{
					if (subDetails->publisherUpdatePollItem.socket == pollIter->socket)
					{
						pollItems.erase(pollIter);
						logger->debug("delete socket from pollitems, size is now {}", pollItems.size());
						break;
					}
				}

				// Remove from subscriptionMap
				DomainDataKey key(subDetails->domain, subDetails->dataProductID);
				subscriptionMap[key].erase(subDetails->filter);
				if (subscriptionMap[key].empty())
				{
					subscriptionMap.erase(key);
				}
			}
			*/
        }
        deleteList.clear();
    }

    // Clean up all our open sockets
    for (map<DomainDataKey, map<std::string, std::shared_ptr<SubscriptionDetails> > >::iterator iter =
             subscriptionMap.begin();
         iter != subscriptionMap.end(); iter++)
    {
        for (map<std::string, std::shared_ptr<SubscriptionDetails> >::iterator detIter = iter->second.begin();
             detIter != iter->second.end(); detIter++)
        {
            for (std::map<std::string, zmq_pollitem_t>::iterator piIter = detIter->second->pollItemMap.begin();
                 piIter != detIter->second->pollItemMap.end(); piIter++)
            {
                zmq_close(piIter->second.socket);
            }
            zmq_close(detIter->second->publisherUpdatePollItem.socket);
        }
    }

    subscriptionMap.clear();
    subscriptionSocketMap.clear();
    socketVerificationMap.clear();
    zmq_close(gravityNodeSocket);
    zmq_close(gravityMetricsSocket);
}

void GravitySubscriptionManager::ready()
{
    // Create the request socket
    void* initSocket = zmq_socket(context, ZMQ_REQ);

    // Connect to service
    zmq_connect(initSocket, "inproc://gravity_init");

    // Send request to service provider
    sendStringMessage(initSocket, "GravitySubscriptionManager", ZMQ_DONTWAIT);

    zmq_close(initSocket);
}

int GravitySubscriptionManager::readSubscription(void* socket, string& filterText,
                                                 std::shared_ptr<GravityDataProduct>& dataProduct)
{
    // Messages
    zmq_msg_t filter, message;

    int ret = 0;

    // Read data products from socket
    zmq_msg_init(&filter);
    ret = zmq_recvmsg(socket, &filter, ZMQ_DONTWAIT);
    if (ret == -1)
    {
        zmq_msg_close(&filter);
        return ret;
    }
    int size = zmq_msg_size(&filter);
    char* s = (char*)malloc(size + 1);
    memcpy(s, zmq_msg_data(&filter), size);
    s[size] = 0;
    filterText = string(s, size);
    free(s);
    zmq_msg_close(&filter);

    zmq_msg_init(&message);
    zmq_recvmsg(socket, &message, 0);
    // Create new GravityDataProduct from the incoming message
    dataProduct =
        std::shared_ptr<GravityDataProduct>(new GravityDataProduct(zmq_msg_data(&message), zmq_msg_size(&message)));
    // Clean up message
    zmq_msg_close(&message);

    return ret;
}

void* GravitySubscriptionManager::setupSubscription(const string& url, const string& filter, zmq_pollitem_t& pollItem)
{
    logger->trace("Setting up subscription for {}", url);
    // Create the socket
    void* subSocket = zmq_socket(context, ZMQ_SUB);
    logger->trace("Created socket");

    // Configure high water mark
    zmq_setsockopt(subSocket, ZMQ_RCVHWM, &subscribeHWM, sizeof(subscribeHWM));
    logger->trace("Configured hwm");

    // Configure filter
    zmq_setsockopt(subSocket, ZMQ_SUBSCRIBE, filter.c_str(), filter.length());
    logger->trace("Configured filter");

    // Connect to publisher
    zmq_connect(subSocket, url.c_str());
    logger->trace("connected to publisher");

    // set up the poll item for this subscription
    pollItem.socket = subSocket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;
    pollItems.push_back(pollItem);
    logger->trace("Added to pollItem");

    return subSocket;
}

void GravitySubscriptionManager::removePollItem(zmq_pollitem_t& pollItem)
{
    // Remove from poll items
    vector<zmq_pollitem_t>::iterator pollIter = pollItems.begin();
    while (pollIter != pollItems.end())
    {
        if (pollIter->socket == pollItem.socket)
        {
            pollIter = pollItems.erase(pollIter);
        }
        else
        {
            pollIter++;
        }
    }
}

void GravitySubscriptionManager::setHWM()
{
    // Read the high water mark setting
    subscribeHWM = readIntMessage(gravityNodeSocket);
}

void GravitySubscriptionManager::addSubscription()
{
    logger->trace("Adding subscription");
    // Read the data product id for this subscription
    string dataProductID = readStringMessage(gravityNodeSocket);
    logger->trace("dataProductID = '{}'", dataProductID);

    bool receiveLastCachedValue = readIntMessage(gravityNodeSocket);
    logger->trace("receiveLastCachedValue = {}", receiveLastCachedValue);

    // Read all the publisher infos
    uint32_t numPubInfoPBs = readUint32Message(gravityNodeSocket);
    list<PublisherInfoPB> pubInfoPBs;
    for (uint32_t i = 0; i < numPubInfoPBs; i++)
    {
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_recvmsg(gravityNodeSocket, &msg, -1);
        PublisherInfoPB pb;
        pb.ParseFromArray(zmq_msg_data(&msg), zmq_msg_size(&msg));
        zmq_msg_close(&msg);
        pubInfoPBs.push_back(pb);
        if (pb.has_url()) logger->trace("url = '{}'", pb.url());
    }

    // Read the subscription filter
    string filter = readStringMessage(gravityNodeSocket);
    logger->trace("filter = {}s'", filter);

    // Read the domain
    string domain = readStringMessage(gravityNodeSocket);
    logger->trace("domain = '{}'", domain);

    // Read the url for to subscribe to for publisher updates
    string publisherUpdateUrl = readStringMessage(gravityNodeSocket);
    logger->trace("publisherUpdateUrl = '{}'", publisherUpdateUrl);

    // Read the subscriber
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(gravityNodeSocket, &msg, -1);
    GravitySubscriber* subscriber;
    memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    logger->trace("Set up GravitySubscriber");

    // Create the domain/data key for tracking subscriptions
    DomainDataKey key(domain, dataProductID);

    // Create new SubscriptionDetails
    std::shared_ptr<SubscriptionDetails> subDetails;

    if (subscriptionMap.count(key) == 0)
    {
        map<string, std::shared_ptr<SubscriptionDetails> > filterMap;
        subscriptionMap[key] = filterMap;
    }

    if (subscriptionMap[key].count(filter) > 0)
    {
        logger->trace("Already have details for this");
        // Already have a details for this
        subDetails = subscriptionMap[key][filter];
    }
    else
    {
        logger->trace("Populate new subDetails");
        subDetails.reset(new SubscriptionDetails());
        subDetails->dataProductID = dataProductID;
        subDetails->domain = domain;
        subDetails->filter = filter;
        subDetails->receiveCachedDataProducts = receiveLastCachedValue;

        zmq_pollitem_t pollItem;
        setupSubscription(publisherUpdateUrl, dataProductID, pollItem);
        subDetails->publisherUpdatePollItem = pollItem;

        subscriptionMap[key][filter] = subDetails;
    }

    list<PublisherInfoPB> trimmedPublishers;
    trimPublishers(pubInfoPBs, trimmedPublishers);
    for (list<PublisherInfoPB>::iterator iter = trimmedPublishers.begin(); iter != trimmedPublishers.end(); iter++)
    {
        // if we have a url and we haven't seen it before, subscribe to it
        if (iter->has_url() && subDetails->pollItemMap.count(iter->url()) == 0)
        {
            logger->trace("Subscribe to new url");
            zmq_pollitem_t pollItem;
            void* subSocket = setupSubscription(iter->url(), filter, pollItem);

            // Create subscription details
            subDetails->pollItemMap[iter->url()] = pollItem;

            // Add lookup from socket to URL
            subDetails->socketToUrlMap[subSocket] = iter->url();

            // and by socket for quick lookup as data arrives
            subscriptionSocketMap[subSocket] = subDetails;

            // Map to manage publisher verification
            socketVerificationMap[subSocket] = iter->registration_time();
        }
    }

    // if a monitor was registered before the subscription, reset the timeouts
    if (subDetails->subscribers.empty() && !subDetails->monitors.empty())
    {
        uint64_t currTime = getCurrentTime() / 1000;
        for (set<std::shared_ptr<TimeoutMonitor> >::iterator monitorIter = subDetails->monitors.begin();
             monitorIter != subDetails->monitors.end(); monitorIter++)
        {
            (*monitorIter)->endTime = currTime + (*monitorIter)->timeout;
        }
    }

    // Add new subscriber if it isn't already in the list
    if (subDetails->subscribers.find(subscriber) == subDetails->subscribers.end())
    {
        logger->trace("Adding new subscriber to list");
        subDetails->subscribers.insert(subscriber);

        // If we've already received data on this subscription, send the most recent
        // value to the new subscriber, unless the subscriber doesn't want the lastCachedValue
        if (receiveLastCachedValue)
        {
            logger->trace("Sending cached value to subscriber");
            vector<std::shared_ptr<GravityDataProduct> > dataProducts;
            for (map<string, zmq_pollitem_t>::iterator iter = subDetails->pollItemMap.begin();
                 iter != subDetails->pollItemMap.end(); iter++)
            {
                if (lastCachedValueMap[iter->second.socket])
                {
                    dataProducts.push_back(lastCachedValueMap[iter->second.socket]);
                }
            }
            if (dataProducts.size() > 0)
            {
                logger->debug("sending data ({}) to late subscriber", dataProductID);
                sort(dataProducts.begin(), dataProducts.end(), sortCacheValues);
                subscriber->subscriptionFilled(dataProducts);
            }
        }
        else
        {
            logger->trace("Not sending cached value to subscriber");
        }
    }
    else
    {
        logger->trace("Subscriber already in list");
    }
}

void GravitySubscriptionManager::removeSubscription()
{
    // Read data product id
    string dataProductID = readStringMessage(gravityNodeSocket);

    // Remove this data product from our metrics
    metricsData.remove(dataProductID);

    // Read the subscription filter
    string filter = readStringMessage(gravityNodeSocket);

    // Read the domain
    string domain = readStringMessage(gravityNodeSocket);

    logger->trace("unsubscribe from data product: {}:{}:{}", domain, dataProductID, filter);

    // Read the subscriber
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(gravityNodeSocket, &msg, -1);
    GravitySubscriber* subscriber;
    memcpy(&subscriber, zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    // Create the domain/data key for tracking subscriptions
    DomainDataKey key(domain, dataProductID);

    if (subscriptionMap.count(key) > 0 && subscriptionMap[key].count(filter) > 0)
    {
        logger->trace("Found at least one subscriber");
        // Get subscription details
        std::shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[key][filter];

        // Find & remove subscriber from our list of subscribers for this data product
        set<GravitySubscriber*>::iterator iter = subDetails->subscribers.begin();
        while (iter != subDetails->subscribers.end())
        {
            // Pointer to same subscriber?
            if (*iter == subscriber)
            {
                logger->trace("Found and removed subscriber");
                subDetails->subscribers.erase(iter);
                break;
            }
            else
            {
                iter++;
            }
        }

        // If no more subscribers, close the subscription sockets and clear the details
        if (subDetails->subscribers.empty())
        {
            logger->trace("No more subscribers");
            //if no more monitor references
            if (subDetails->monitors.empty())
            {
                logger->trace("No more monitors");

                // Remove details from main map
                subscriptionMap[key].erase(filter);
                if (subscriptionMap[key].size() == 0)
                {
                    subscriptionMap.erase(key);
                }

                // If we no longer have subscriptions for this domain/dataProductID combo, then stop
                // subscribing for updates from the SD on this as well
                logger->trace("Removing subscription to {}", gravity::constants::REGISTERED_PUBLISHERS_DPID);
                removePollItem(subDetails->publisherUpdatePollItem);
                zmq_setsockopt(subDetails->publisherUpdatePollItem.socket, ZMQ_UNSUBSCRIBE, dataProductID.c_str(),
                               dataProductID.length());
                zmq_close(subDetails->publisherUpdatePollItem.socket);
            }

            for (map<string, zmq_pollitem_t>::iterator iter = subDetails->pollItemMap.begin();
                 iter != subDetails->pollItemMap.end(); iter++)
            {
                unsubscribeFromPollItem(iter->second, filter);

                // Remove from poll items
                removePollItem(iter->second);

                // Clear cached values
                lastCachedValueMap.erase(iter->second.socket);
            }
        }
    }
}

void GravitySubscriptionManager::setTimeoutMonitor()
{
    string dataProductID = readStringMessage(gravityNodeSocket);

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(gravityNodeSocket, &msg, -1);
    GravitySubscriptionMonitor* monitor;
    memcpy(&monitor, zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    int timeout = readIntMessage(gravityNodeSocket);

    string filter = readStringMessage(gravityNodeSocket);
    string domain = readStringMessage(gravityNodeSocket);

    DomainDataKey key(domain, dataProductID);

    std::shared_ptr<SubscriptionDetails> subDetails;
    if (subscriptionMap[key].count(filter) > 0)
    {
        // Already have a details for this
        subDetails = subscriptionMap[key][filter];
    }
    else
    {
        subDetails.reset(new SubscriptionDetails());
        subDetails->dataProductID = dataProductID;
        subDetails->domain = domain;
        subDetails->filter = filter;
        map<string, std::shared_ptr<SubscriptionDetails> > filterMap;
        subscriptionMap[key] = filterMap;
        subscriptionMap[key][filter] = subDetails;
    }

    uint64_t currTime = getCurrentTime() / 1000;

    for (std::set<std::shared_ptr<TimeoutMonitor> >::iterator iter = subDetails->monitors.begin();
         iter != subDetails->monitors.end(); iter++)
    {
        // if a reference to this monitor already exists
        if ((*iter)->monitor == monitor)
        {
            (*iter)->timeout = timeout;
            (*iter)->endTime = currTime + timeout;
            return;
        }
    }

    // create and add new monitor
    std::shared_ptr<TimeoutMonitor> tm;
    tm.reset(new TimeoutMonitor());

    tm->monitor = monitor;
    tm->timeout = timeout;
    tm->endTime = currTime + timeout;
    tm->lastReceived = -1l;

    subDetails->monitors.insert(tm);
}

void GravitySubscriptionManager::clearTimeoutMonitor()
{
    string dataProductID = readStringMessage(gravityNodeSocket);

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(gravityNodeSocket, &msg, -1);
    GravitySubscriptionMonitor* monitor;
    memcpy(&monitor, zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    string filter = readStringMessage(gravityNodeSocket);
    string domain = readStringMessage(gravityNodeSocket);

    DomainDataKey key(domain, dataProductID);

    if (subscriptionMap.count(key) > 0 && subscriptionMap[key].count(filter) > 0)
    {
        std::shared_ptr<SubscriptionDetails> subDetails = subscriptionMap[key][filter];

        // Find & remove all matching monitors
        set<std::shared_ptr<TimeoutMonitor> >::iterator iter = subDetails->monitors.begin();
        while (iter != subDetails->monitors.end())
        {
            if ((*iter)->monitor == monitor)
            {
                //Is This necessary?
                if ((*iter) == currTimeoutMonitor)
                {
                    currTimeoutMonitor.reset();
                    pollTimeout = -1;
                }
                subDetails->monitors.erase(iter);
                break;
            }

            iter++;
        }

        //if no more references to this subscription, remove from map
        if (subDetails->monitors.empty() && subDetails->subscribers.empty())
        {
            // Remove from details main map
            subscriptionMap[key].erase(filter);
            if (subscriptionMap[key].size() == 0)
            {
                subscriptionMap.erase(key);
            }
            // If we no longer have subscriptions for this domain/dataProductID combo, then stop
            // subscribing for updates from the SD on this as well
            removePollItem(subDetails->publisherUpdatePollItem);
            zmq_setsockopt(subDetails->publisherUpdatePollItem.socket, ZMQ_UNSUBSCRIBE, dataProductID.c_str(),
                           dataProductID.length());
            zmq_close(subDetails->publisherUpdatePollItem.socket);
        }
    }
}

void GravitySubscriptionManager::unsubscribeFromPollItem(zmq_pollitem_t pollItem, string filterText)
{
    // Clean-up/Subscribe from this subscription
    subscriptionSocketMap.erase(pollItem.socket);
    socketVerificationMap.erase(pollItem.socket);

    // Unsubscribe
    zmq_setsockopt(pollItem.socket, ZMQ_UNSUBSCRIBE, filterText.c_str(), filterText.length());

    // Close the socket
    zmq_close(pollItem.socket);
}

void GravitySubscriptionManager::calculateTimeout()
{
    int minTime = -1;
    currTimeoutMonitor.reset();
    currMonitorDetails.reset();
    map<DomainDataKey, map<std::string, std::shared_ptr<SubscriptionDetails> > >::iterator dpIter =
        subscriptionMap.begin();
    //go through all domain-dataproduct keys
    while (dpIter != subscriptionMap.end())
    {
        map<string, std::shared_ptr<SubscriptionDetails> >::iterator filterIter = dpIter->second.begin();
        while (filterIter != dpIter->second.end())
        {
            std::shared_ptr<SubscriptionDetails> subDetails = filterIter->second;
            for (set<std::shared_ptr<TimeoutMonitor> >::iterator monitorIter = subDetails->monitors.begin();
                 monitorIter != subDetails->monitors.end(); monitorIter++)
            {
                // check if this is an active subscription with a valid timeout
                if (subDetails->subscribers.size() > 0 && (*monitorIter)->timeout >= 0)
                {
                    uint64_t currTime = getCurrentTime() / 1000;
                    // calculate how much time is left until this monitor times out
                    int64_t timeRemaining = (*monitorIter)->endTime - currTime;

                    //a subscription timed out during processing
                    if (timeRemaining <= 0)
                    {
                        int timeSinceLast =
                            (*monitorIter)->lastReceived > 0 ? (int)(currTime - (*monitorIter)->lastReceived) : -1l;
                        //make call to monitor
                        (*monitorIter)
                            ->monitor->subscriptionTimeout(subDetails->dataProductID, timeSinceLast, subDetails->filter,
                                                           subDetails->domain);
                        //reset next timeout
                        (*monitorIter)->endTime = (*monitorIter)->endTime + (*monitorIter)->timeout;

                        logger->trace("Subscription Timeout ({})", subDetails->dataProductID);

                        if ((*monitorIter)->timeout < minTime || minTime == -1)
                        {
                            minTime = (int)(*monitorIter)->timeout;
                        }
                    }
                    else if (timeRemaining < (int64_t)minTime || minTime == -1)
                    {
                        //set current timeout details
                        minTime = (int)timeRemaining;
                        currTimeoutMonitor = *monitorIter;
                        currMonitorDetails = subDetails;
                    }
                }
            }
            filterIter++;
        }
        dpIter++;
    }
    pollTimeout = minTime;
}

void GravitySubscriptionManager::notifyServiceDirectoryOfStaleEntry(string dataProductId, string domain, string url,
                                                                    uint32_t regTime)
{
    logger->debug("Notifying ServiceDirectory of stale publisher: [{} @ {}]", dataProductId, url);
    ServiceDirectoryUnregistrationPB unregistration;
    unregistration.set_id(dataProductId);
    unregistration.set_url(url);
    unregistration.set_domain(domain);
    unregistration.set_registration_time(regTime);
    unregistration.set_type(ServiceDirectoryUnregistrationPB::DATA);

    GravityDataProduct request("UnregistrationRequest");
    request.setData(unregistration);

    void* socket = zmq_socket(context, ZMQ_REQ);  // Socket to connect to service provider
    zmq_connect(socket, serviceDirectoryUrl.c_str());
    int linger = -1;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    // Send message to service provider
    sendGravityDataProduct(socket, request, ZMQ_DONTWAIT);

    zmq_close(socket);
}

void GravitySubscriptionManager::collectMetrics(vector<std::shared_ptr<GravityDataProduct> > dataProducts)
{
    // Iterate over all the data products
    vector<std::shared_ptr<GravityDataProduct> >::iterator gdpIter;
    for (gdpIter = dataProducts.begin(); gdpIter != dataProducts.end(); gdpIter++)
    {
        std::shared_ptr<GravityDataProduct> gdp = *gdpIter;
        metricsData.incrementMessageCount(gdp->getDataProductID(), 1);
        metricsData.incrementByteCount(gdp->getDataProductID(), gdp->getSize());
    }
}

/**
 * This assumes publishers are already for the same data product id and domain
 */
void GravitySubscriptionManager::trimPublishers(const std::list<gravity::PublisherInfoPB>& fullList,
                                                std::list<gravity::PublisherInfoPB>& trimmedList)
{
    bool iAmRelay = false;
    for (list<PublisherInfoPB>::const_iterator iter = fullList.begin(); iter != fullList.end(); iter++)
    {
        // if this is a valid publisher and we are a relay, then don't want to subscribe to one
        if (iter->has_url() && iter->isrelay() && iter->componentid() == componentID)
        {
            iAmRelay = true;
        }
    }

    trimmedList.clear();
    bool foundGlobalRelay = false;
    for (list<PublisherInfoPB>::const_iterator iter = fullList.begin(); iter != fullList.end(); iter++)
    {
        if (!iter->has_url())  // if invalid publisher - should never happen
        {
            logger->warn("Found publisher with no url??  Skipping...");
            continue;
        }
        else if (iAmRelay)
        {
            if (!iter->isrelay()) trimmedList.push_back(*iter);
        }
        else if (iter->isrelay())
        {
            if (iter->has_ipaddress())
            {
                logger->trace("found valid relay with ip = {}, my ip = {}", iter->ipaddress(), ipAddress);
            }
            else
            {
                logger->trace("Found valid global relay");
            }
            // if it's a relay for any IP or our IP
            if (!iter->has_ipaddress() || iter->ipaddress() == ipAddress)
            {
                trimmedList.clear();
                trimmedList.push_back(*iter);
                if (!iter->has_ipaddress())
                    // we'll keep looking for a local relay if we've found a global one
                    foundGlobalRelay = true;
                else
                    // we have a local relay, so we're done here
                    break;
            }
            else  // ignore it
            {
                continue;
            }
        }

        // The normal case
        if (!iAmRelay && !foundGlobalRelay)
        {
            trimmedList.push_back(*iter);
        }
    }
    logger->trace("added {} elements to trimmed pub list", trimmedList.size());
}

} /* namespace gravity */
