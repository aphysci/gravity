#include <iostream>
#include <queue>
#include <list>
#include <map>
#include <set>
#include <sstream>

#include <zmq.h>
#include "GravityNode.h"
#include "GravityHeartbeatListener.h"
#include "GravityHeartbeat.h"
#include "GravitySemaphore.h"

namespace gravity
{

using namespace std;

//Comparison so the Heap works correctly.
bool operator< (const ExpectedMessasgeQueueElement &a, ExpectedMessasgeQueueElement &b)
{
	return a.expectedTime < b.expectedTime;
}


std::list<ExpectedMessasgeQueueElement> Heartbeat::queueElements; //This is so we can reuse these guys.
std::priority_queue<ExpectedMessasgeQueueElement*> Heartbeat::messageTimes;
std::map<std::string, GravityHeartbeatListener*> Heartbeat::listener;

Semaphore Heartbeat::lock;
std::set<std::string> Heartbeat::filledHeartbeats;


void Heartbeat::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	lock.Lock();
	filledHeartbeats.insert(dataProduct.getDataProductID());
	lock.Unlock();
}


void* Heartbeat::HeartbeatListenerThrFunc(void* thread_context)
{
	HBListenerContext* params = (HBListenerContext*) thread_context;

    void* hbSocket = zmq_socket(params->zmq_context, ZMQ_REP);
    zmq_connect(hbSocket, "inproc://heartbeat_listener");

    while(true)
    {
    	if(!messageTimes.empty())
    	{
    		zmq_msg_t msg;
    	    zmq_msg_init(&msg);

			ExpectedMessasgeQueueElement& mqe = *messageTimes.top();
#ifdef WIN32
			Sleep((mqe.expectedTime - getCurrentTime())/1000);
#else
			struct timespec request;
			request.tv_sec = mqe.expectedTime / 1000000;
			request.tv_nsec = (mqe.expectedTime % 1000000)*1000; //Convert from Microseconds to Nanoseconds
			clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &request, NULL);
#endif

			lock.Lock();
			std::set<std::string>::iterator i = filledHeartbeats.find(mqe.dataproductID);
			bool gotHeartbeat = (i != filledHeartbeats.end());
			if(gotHeartbeat)
				filledHeartbeats.erase(i);
			lock.Unlock();

			if(!gotHeartbeat)
			{
				listener[mqe.dataproductID]->MissedHeartbeat(mqe.dataproductID, getCurrentTime() - mqe.lastHeartbeatTime, "Missed");
			}
			else
				mqe.lastHeartbeatTime = getCurrentTime();


			messageTimes.pop();
			mqe.expectedTime = getCurrentTime() + mqe.timetowaitBetweenHeartbeats; //(Maybe should be lastHeartbeatTime + timetowaitBetweenHeartbeats, but current version allows for drift etc.)
			messageTimes.push(&mqe);
    	}//Process Messages

        //Allow Gravity to add Heartbeat listeners.
    	{
    		zmq_msg_t msg;
    	    zmq_msg_init(&msg);
    	    std::string dataproductID;
    	    //unsigned short port;
    	    uint64_t maxtime;

    	    if(zmq_recvmsg(hbSocket, &msg, ZMQ_DONTWAIT) != -1)
    	    {
				//Receive Dataproduct ID
				int size = zmq_msg_size(&msg);
				char* s = (char*)malloc(size+1);
				memcpy(s, zmq_msg_data(&msg), size);
				s[size] = 0;
				dataproductID.assign(s, size);
				free(s);
				//zmq_msg_close(&msg); //Closed after the end of the if statement.

				//Receive address of listener
				zmq_msg_t msg2;
				zmq_msg_init(&msg2);
				intptr_t p;
				zmq_recvmsg(hbSocket, &msg2, ZMQ_DONTWAIT); //These are guarunteed to not fail since this is a multi part message.
				memcpy(&p, zmq_msg_data(&msg2), sizeof(intptr_t));
				zmq_msg_close(&msg2);

				listener[dataproductID] = (GravityHeartbeatListener*) p;

				//Receive maxtime.
				zmq_msg_t msg3;
				zmq_msg_init(&msg3);
				zmq_recvmsg(hbSocket, &msg3, ZMQ_DONTWAIT);
				memcpy(&maxtime, zmq_msg_data(&msg3), 8);
				zmq_msg_close(&msg3);

				ExpectedMessasgeQueueElement mqe1;
				mqe1.dataproductID = dataproductID;
				mqe1.expectedTime = getCurrentTime() + maxtime;
				mqe1.lastHeartbeatTime = 0;
				mqe1.timetowaitBetweenHeartbeats = maxtime;

				//We should already be subscribed to the Heartbeats.

				queueElements.push_back(mqe1);
				messageTimes.push(&queueElements.back());
    	    }

	    	zmq_msg_close(&msg);

    	}//Add Heartbeat listeners

    }//while(true)

	//This will never be reached but should be done when the thread ends.
    delete params;

    return NULL;
}

void* Heartbeat(void* thread_context)
{
	HBParams* params = (HBParams*) thread_context;
	GravityNode* gn = (GravityNode*) params->gn;
	GravityDataProduct gdp(params->componentID);
	gdp.setData((void*)"Good", 5);

	while(true)
	{
		gn->publish(gdp);
#ifdef WIN32
		Sleep(params->interval_in_microseconds/1000);
#else
		struct timespec request;
		request.tv_sec = params->interval_in_microseconds / 1000000;
		request.tv_nsec = (params->interval_in_microseconds % 1000000)*1000; //Convert from Microseconds to Nanoseconds
		clock_nanosleep(CLOCK_REALTIME, 0, &request, NULL);
#endif
	}

	//This will never be reached but should be done when the thread ends.
	delete params;

	return NULL;
}

} //namespace gravity
