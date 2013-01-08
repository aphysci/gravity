#ifndef GRAVITY_HEARTBEAT_H__
#define GRAVITY_HEARTBEAT_H__
//Internal Header for Gravity Heartbeat related things.
#include <iostream>
#include <queue>
#include <list>
#include <map>
#include <set>
#include <sstream>

#include <zmq.h>
#include "GravityNode.h"
#include "GravityHeartbeatListener.h"
#include "GravitySemaphore.h"

namespace gravity
{
//For passing Data to the HB threads.
struct HBParams {
	void* zmq_context;
	int interval_in_microseconds;
	//unsigned short port;
	std::string componentID;
	std::string endpoint;
	int minPort;
	int maxPort;
};

struct HBListenerContext {
	void* zmq_context;
};

struct ExpectedMessageQueueElement {
	uint64_t expectedTime; //Absolute (Maximum amount we can wait).
	uint64_t lastHeartbeatTime; //Absolute
	uint64_t timetowaitBetweenHeartbeats; //In Microseconds
	std::string dataproductID;
	void* socket;
};

struct EMQComparator
{
	bool operator() (ExpectedMessageQueueElement* a, ExpectedMessageQueueElement* b)
	{
		return a->expectedTime > b->expectedTime;
	}
};

class Heartbeat : public GravitySubscriber
{
public:
    virtual void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts);

    static void* HeartbeatListenerThrFunc(void* thread_context);

    static std::list<ExpectedMessageQueueElement> queueElements; //This is so we can reuse these guys.
    static std::priority_queue<ExpectedMessageQueueElement*, vector<ExpectedMessageQueueElement*>, EMQComparator> messageTimes;
    static std::map<std::string, GravityHeartbeatListener*> listener;

    static Semaphore lock;
    static std::set<std::string> filledHeartbeats;
};

} //namespace gravity

#endif //GRAVITY_HEARTBEAT_H__
