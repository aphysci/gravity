/*
 * GravitySubscriptionManager.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYPUBLISHMANAGER_H_
#define GRAVITYPUBLISHMANAGER_H_

#include "Utility.h"

#ifdef __GNUC__
#include <tr1/memory>
#else
#include <memory>
#endif
#include <zmq.h>
#include <vector>
#include <map>
#include <string>

#define PUB_MGR_REQ_URL "inproc://gravity_publish_manager_request"
#define PUB_MGR_PUB_URL "inproc://gravity_publish_manager_publish"

namespace gravity
{

using namespace std;
using namespace std::tr1;

typedef struct CacheValue
{
    string filterText;
    char *value;
    int size;
    uint64_t timestamp;
} CacheValue;

typedef struct PublishDetails
{
    string url;
    string dataProductID;
    map<string,shared_ptr<CacheValue> > lastCachedValues;
    zmq_pollitem_t pollItem;
    void* socket;
} PublishDetails;

/**
 * The GravityPublishManager is a component used internally by the GravityNode to allow
 * late subscribers to receive the most recent value that they missed.
 */
class GravityPublishManager
{
private:
	void* context;
	void* gravityNodeResponseSocket;
    void* gravityNodeSubscribeSocket;
    map<void*,shared_ptr<PublishDetails> > publishMapBySocket;
    map<string,shared_ptr<PublishDetails> > publishMapByID;
	vector<zmq_pollitem_t> pollItems;

	void ready();
	void registerDataProduct();
	void unregisterDataProduct();
	void publish();
    void publish(void* socket, const string &filterText, const void *data, int size);
public:
	/**
	 * Constructor GravityPublishManager
	 * \param context The zmq context in which the inproc socket will be established with the GravityNode
	 */
	GravityPublishManager(void* context);

	/**
	 * Default destructor
	 */
	virtual ~GravityPublishManager();

	/**
	 * Starts the GravityPublishManager which will run forever, sending updates to new subscribers.
	 * Should be executed from GravityNode in its own thread with a shared zmq context.
	 */
	void start();
};

} /* namespace gravity */
#endif /* GRAVITYPUBLISHMANAGER_H_ */
