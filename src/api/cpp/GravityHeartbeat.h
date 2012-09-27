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

namespace gravity
{
//For passing Data to the HB threads.
struct HBParams {
	void* zmq_context;
	int interval_in_microseconds;
	//unsigned short port;
	std::string componentID;
	void* gn;
};

struct HBListenerContext {
	void* zmq_context;
	void* gn;
};

struct ExpectedMessasgeQueueElement {
	uint64_t expectedTime; //Absolute (Maximum amount we can wait).
	uint64_t lastHeartbeatTime; //Absolute
	uint64_t timetowaitBetweenHeartbeats; //In Microseconds
	std::string dataproductID;
	void* socket;
};

bool operator< (const ExpectedMessasgeQueueElement &a, ExpectedMessasgeQueueElement &b);

class ZMQSemephore
{
public:
	static void init(void* context);

	ZMQSemephore();
	void Lock();
	void Unlock();
	~ZMQSemephore();
private:
	void* socket;
	static int num;
	static void* zmq_context;
};

class Heartbeat : public GravitySubscriber
{
public:
    virtual void subscriptionFilled(const GravityDataProduct& dataProduct);

    static void* HeartbeatListenerThrFunc(void* thread_context);

    static std::list<ExpectedMessasgeQueueElement> queueElements; //This is so we can reuse these guys.
    static std::priority_queue<ExpectedMessasgeQueueElement*> messageTimes;
    static std::map<std::string, GravityHeartbeatListener*> listener;

    static ZMQSemephore lock;
    static std::set<std::string> filledHeartbeats;
};

} //namespace gravity

#endif //GRAVITY_HEARTBEAT_H__
