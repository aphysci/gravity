/*
 * GravitySubscriptionManager.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYPUBLISHMANAGER_H_
#define GRAVITYPUBLISHMANAGER_H_

#include <zmq.h>
#include <vector>
#include <map>
#include <tr1/memory>
#include <string>

namespace gravity
{

using namespace std;
using namespace std::tr1;

typedef struct PublishDetails
{
    string url;
    string dataProductID;
    char *lastCachedValue;
    int lastCachedValueSize;
    string lastCachedFilterText;
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
	void* gravityNodeSocket;
    map<void*,shared_ptr<PublishDetails> > publishMapBySocket;
    map<string,shared_ptr<PublishDetails> > publishMapByID;
	vector<zmq_pollitem_t> pollItems;

	string readStringMessage();
	void sendStringMessage(void* socket, string str, int flags);
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
