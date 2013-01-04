/*
 * GravityPublishManager.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Mark Barger
 */

#include "GravityPublishManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include "zmq.h"
#include <sstream>
#include <algorithm>

namespace gravity
{

using namespace std;

bool sortCacheValues (const shared_ptr<CacheValue> &i, const shared_ptr<CacheValue> &j)
{
    return i->timestamp < j->timestamp;
}

GravityPublishManager::GravityPublishManager(void* context)
{
	// This is the zmq context that is shared with the GravityNode. Must use
	// a shared context to establish an inproc socket.
	this->context = context;
}

GravityPublishManager::~GravityPublishManager() {}

void GravityPublishManager::start()
{
	// Messages
	zmq_msg_t event, id;

	// Set up the inproc sockets to subscribe and unsubscribe to messages from
	// the GravityNode
	gravityNodeResponseSocket = zmq_socket(context, ZMQ_REP);
	zmq_bind(gravityNodeResponseSocket, PUB_MGR_REQ_URL);

	gravityNodeSubscribeSocket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(gravityNodeSubscribeSocket, PUB_MGR_PUB_URL);
    zmq_setsockopt(gravityNodeSubscribeSocket, ZMQ_SUBSCRIBE, NULL, 0);

	// Always have at least the gravity node to poll
	zmq_pollitem_t pollItemResponse;
	pollItemResponse.socket = gravityNodeResponseSocket;
	pollItemResponse.events = ZMQ_POLLIN;
	pollItemResponse.fd = 0;
	pollItemResponse.revents = 0;
	pollItems.push_back(pollItemResponse);

    // Always have at least the gravity node to poll
    zmq_pollitem_t pollItemSubscribe;
    pollItemSubscribe.socket = gravityNodeSubscribeSocket;
    pollItemSubscribe.events = ZMQ_POLLIN;
    pollItemSubscribe.fd = 0;
    pollItemSubscribe.revents = 0;
    pollItems.push_back(pollItemSubscribe);

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

		// Process new requests from the gravity node
		if (pollItems[0].revents & ZMQ_POLLIN)
		{
			// Get new GravityNode request
			string command = readStringMessage(gravityNodeResponseSocket);

			// message from gravity node should be either a register, unregister or publish request
			if (command == "register")
			{
				registerDataProduct();
			}
			else
			{
			    Log::critical("Received unknown request command %s", command.c_str());
			}
		}

		if (pollItems[1].revents & ZMQ_POLLIN)
		{
            // Get new GravityNode request
            string command = readStringMessage(gravityNodeSubscribeSocket);

			if (command == "unregister")
			{
				unregisterDataProduct();
			}
			else if (command == "publish")
			{
				publish();
			}
			else if (command == "kill")
			{
				break;
			}
			else
			{
                Log::critical("Received unknown publish command %s", command.c_str());
			}
		}

		// Check for publish updates
		for (unsigned int i = 2; i < pollItems.size(); i++)
		{
			if (pollItems[i].revents && ZMQ_POLLIN)
			{
				// Read whether it's a new subscription from socket
				zmq_msg_init(&event);
				zmq_recvmsg(pollItems[i].socket, &event, 0);
				char *data = (char*)zmq_msg_data(&event);
				bool newsub = *((char*)zmq_msg_data(&event)) == 1;
				zmq_msg_close(&event);

				if (newsub)
				{
				    shared_ptr<PublishDetails> pd = publishMapBySocket[pollItems[i].socket];
				    // can't log here because the network logging uses this code - any logs here will result in an
				    // infinite loop, or a deadlock.
				    // This message can be useful though, so leaving it in, but commented out.
//				    Log::debug("got a new subscriber for %s, resending %d values", pd->dataProductID.c_str(), pd->lastCachedValues.size());

				    list<shared_ptr<CacheValue> > values;
				    for (map<string,shared_ptr<CacheValue> >::iterator iter = pd->lastCachedValues.begin(); iter != pd->lastCachedValues.end(); iter++)
				        values.push_back(iter->second);

				    // we shouldn't be doing this often, so just sort these when we need it.
                    values.sort(sortCacheValues);
				    for (list<shared_ptr<CacheValue> >::iterator iter = values.begin(); iter != values.end(); iter++)
				        publish(pd->socket, (*iter)->filterText, (*iter)->value, (*iter)->size);
				}
			}
		}
	}

	// Clean up any pub sockets
	for (map<void*,shared_ptr<PublishDetails> >::iterator iter = publishMapBySocket.begin(); iter != publishMapBySocket.end(); iter++)
	{
		shared_ptr<PublishDetails> pubDetails = publishMapBySocket[iter->second->socket];
		zmq_close(pubDetails->pollItem.socket);
        for (map<string,shared_ptr<CacheValue> >::iterator valIter = pubDetails->lastCachedValues.begin(); valIter != pubDetails->lastCachedValues.end(); valIter++)
            delete [] valIter->second->value;
        pubDetails->lastCachedValues.clear();
	}

	publishMapBySocket.clear();
	publishMapByID.clear();

    zmq_close(gravityNodeResponseSocket);
	zmq_close(gravityNodeSubscribeSocket);
}

void GravityPublishManager::ready()
{
	// Create the request socket
	void* initSocket = zmq_socket(context, ZMQ_REQ);

	// Connect to service
	zmq_connect(initSocket, "inproc://gravity_init");

	// Send request to service provider
	sendStringMessage(initSocket, "GravityPublishManager", ZMQ_DONTWAIT);

	zmq_close(initSocket);
}

void GravityPublishManager::registerDataProduct()
{

	// Read the data product id for this request
	string dataProductID = readStringMessage(gravityNodeResponseSocket);

	// Read the publish transport type
	string transportType = readStringMessage(gravityNodeResponseSocket);

    int minPort, maxPort;
    if(transportType == "tcp")
    {
        minPort = readIntMessage(gravityNodeResponseSocket);
        maxPort = readIntMessage(gravityNodeResponseSocket);
    }

    // Read the publish transport type
    string endpoint = readStringMessage(gravityNodeResponseSocket);

    // Create the publish socket
    void* pubSocket = zmq_socket(context, ZMQ_XPUB);
    if (!pubSocket)
    {
        return;
    }
    int verbose = 1;
    zmq_setsockopt(pubSocket, ZMQ_XPUB_VERBOSE, &verbose, sizeof(verbose));

    string connectionURL;
    if(transportType == "tcp")
    {
        int port = bindFirstAvailablePort(pubSocket, endpoint, minPort, maxPort);
        if (port < 0)
        {
            Log::critical("Could not find available port for %s", dataProductID.c_str());
            zmq_close(pubSocket);
            sendStringMessage(gravityNodeResponseSocket, "", ZMQ_DONTWAIT);
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
        int rc = zmq_bind(pubSocket, ss.str().c_str());
        if (rc < 0)
        {
            Log::critical("Could not bind address %s", connectionURL.c_str());
            zmq_close(pubSocket);
            sendStringMessage(gravityNodeResponseSocket, "", ZMQ_DONTWAIT);
            return;
        }
    }

    sendStringMessage(gravityNodeResponseSocket, connectionURL, ZMQ_DONTWAIT);

	// Create poll item for response to this request
	zmq_pollitem_t pollItem;
	pollItem.socket = pubSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

    // Track dataProductID->socket mapping
    shared_ptr<PublishDetails> publishDetails = shared_ptr<PublishDetails>(new PublishDetails);
    publishDetails->url = connectionURL;
    publishDetails->dataProductID = dataProductID;
    publishDetails->socket = pubSocket;
	publishDetails->pollItem = pollItem;

    publishMapByID[dataProductID] = publishDetails;
    publishMapBySocket[pubSocket] = publishDetails;
}

void GravityPublishManager::unregisterDataProduct()
{
	// Read the data product id for this request
	string dataProductID = readStringMessage(gravityNodeSubscribeSocket);

	// If data product ID exists, clean up and remove socket. Otherwise, likely a duplicate unregister request
	if (publishMapByID.count(dataProductID))
	{
		shared_ptr<PublishDetails> publishDetails = publishMapByID[dataProductID];
		void* socket = publishDetails->pollItem.socket;
		publishMapBySocket.erase(socket);
		publishMapByID.erase(dataProductID);
		zmq_unbind(socket, publishDetails->url.c_str());
		zmq_close(socket);

		// delete any cached values.
        for (map<string,shared_ptr<CacheValue> >::iterator iter = publishDetails->lastCachedValues.begin(); iter != publishDetails->lastCachedValues.end(); iter++)
		    delete [] iter->second->value;
        publishDetails->lastCachedValues.clear();

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

void GravityPublishManager::publish()
{
    // Read the filter text
    string filterText = readStringMessage(gravityNodeSubscribeSocket);

    // Read the data product
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(gravityNodeSubscribeSocket, &msg, -1);
    GravityDataProduct dataProduct(zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    shared_ptr<PublishDetails> publishDetails = publishMapByID[dataProduct.getDataProductID()];
    if (!publishDetails)
        return;

    // Serialize data
    int size = dataProduct.getSize();
    char *bytes = new char[size];
    dataProduct.serializeToArray(bytes);

    // delete any old data and ...
    if (publishDetails->lastCachedValues.count(filterText) > 0)
        delete [] publishDetails->lastCachedValues[filterText]->value;

    // ... save new data for late subscribers
    shared_ptr<CacheValue> val = shared_ptr<CacheValue>(new CacheValue);
    val->filterText = filterText;
    val->value = bytes;
    val->size = size;
    val->timestamp = dataProduct.getGravityTimestamp();
    publishDetails->lastCachedValues[filterText] = val;

    publish(publishDetails->socket, filterText, bytes, size);
}

void GravityPublishManager::publish(void* socket, const string &filterText, const void *bytes, int size)
{

    // Create message & send filter text
    sendStringMessage(socket, filterText, ZMQ_SNDMORE);

    zmq_msg_t data;
    zmq_msg_init_size(&data, size);
    memcpy(zmq_msg_data(&data), bytes, size);

    // Publish data
    zmq_sendmsg(socket, &data, ZMQ_DONTWAIT);

    // Clean up
    zmq_msg_close(&data);

}
} /* namespace gravity */
