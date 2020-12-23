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
/** For passing Data to the HB threads. */
struct HBParams {
	void* zmq_context;
	uint64_t interval_in_microseconds;
	//unsigned short port;
	std::string componentID;
	std::string endpoint;
	int minPort;
	int maxPort;
	uint32_t registrationTime;
};

struct HBListenerContext {
	void* zmq_context;
};

struct ExpectedMessageQueueElement {
	uint64_t expectedTime; //Absolute (Maximum amount we can wait).
	uint64_t lastHeartbeatTime; //Absolute
	int64_t timetowaitBetweenHeartbeats; //In Microseconds
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

/**
 * Class that monitors the subscription status (heartbeats) of subscribed data products.
 */
class Heartbeat : public GravitySubscriber
{
private:
	static Semaphore heartbeatLock;
	static bool heartbeatRunning;
public:
    virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);

    static void* HeartbeatListenerThrFunc(void* thread_context);

    static std::list<ExpectedMessageQueueElement> queueElements; //This is so we can reuse these guys.
    static std::priority_queue<ExpectedMessageQueueElement*, std::vector<ExpectedMessageQueueElement*>, EMQComparator> messageTimes;
    static std::map<std::string, GravityHeartbeatListener*> listener;

    static Semaphore lock;

    /**
     * Set of data product IDs that we have received subscriptions (heartbeats) for
     */
    static std::set<std::string> filledHeartbeats;	

	static void setHeartbeatRunning(bool running);
	static bool isHeartbeatRunning();
};

} //namespace gravity

#endif //GRAVITY_HEARTBEAT_H__
