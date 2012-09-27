#include <iostream>
#include <queue>
#include <list>
#include <map>
#include <set>

#include <zmq.h>
#include "GravityNode.h"
#include "GravityHeartbeatListener.h"

namespace gravity
{

using namespace std;

struct ExpectedMessasgeQueueElement {
	uint64_t expectedTime; //Absolute (Maximum amount we can wait).
	uint64_t lastHeartbeatTime; //Absolute
	uint64_t timetowaitBetweenHeartbeats; //In Microseconds
	std::string dataproductID;
	void* socket;
};

//Comparison so the Heap works correctly.
bool operator< (const ExpectedMessasgeQueueElement &a, ExpectedMessasgeQueueElement &b)
{
	return a.expectedTime < b.expectedTime;
}

struct HBListenerContext {
	void* zmq_context;
	void* gn; //Not used.
};

class Heartbeat : public GravitySubscriber
{
public:
    virtual void subscriptionFilled(const GravityDataProduct& dataProduct);

    static void* HeartbeatListenerThrFunc(void* thread_context);

    static list<ExpectedMessasgeQueueElement> queueElements; //This is so we can reuse these guys.
    static priority_queue<ExpectedMessasgeQueueElement*> messageTimes;
    static map<std::string, GravityHeartbeatListener*> listener;

    //Need a lock.
    std::set<std::string> filledHeartbeats;
};

void Heartbeat::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Lock
	filledHeartbeats.insert(dataProduct.getDataProductID());
	//Unlock
}


void* Heartbeat::HeartbeatListenerThrFunc(void* thread_context)
{
	HBListenerContext* params = (HBListenerContext*) thread_context;

    void* hbSocket = zmq_socket(params->zmq_context, ZMQ_REP);
    zmq_bind(hbSocket, "inproc://heartbeat_listener");

    while(true)
    {
    	if(!messageTimes.empty())
    	{
    		zmq_msg_t msg;
    	    zmq_msg_init(&msg);

			ExpectedMessasgeQueueElement& mqe = *messageTimes.top();
#ifdef WIN32
			Sleep((mqe.expectedTime - GetCurrentTime())/1000);
#else
			struct timespec request;
			request.tv_sec = mqe.expectedTime / 1000000;
			request.tv_nsec = (mqe.expectedTime % 1000000)*1000; //Convert from Microseconds to Nanoseconds
			clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, request, NULL);
#endif

			if(zmq_recvmsg(mqe.socket, &msg, ZMQ_DONTWAIT) != 0)
				if(zmq_errno() == EAGAIN)
					listener[mqe.dataproductID]->MissedHeartbeat(mqe.dataproductID, mqe.lastHeartbeatTime - getCurrentTime(), "Missed");
				else
					listener[mqe.dataproductID]->MissedHeartbeat(mqe.dataproductID, mqe.lastHeartbeatTime - getCurrentTime(), "SocketError");
			else
			{
				if(zmq_msg_size(&msg) != 4 && strncmp((char*)zmq_msg_data(&msg), "Good", 4) != 0)
					listener[mqe.dataproductID]->MissedHeartbeat(mqe.dataproductID, mqe.lastHeartbeatTime - getCurrentTime(), string((const char*)zmq_msg_data(&msg), zmq_msg_size(&msg)));
				else
				{
					//Receive any other extraneous heartbeats we may have missed.  //TODO: check status.
					int rc;
					do
					{
			    		zmq_msg_t msg;
			    	    zmq_msg_init(&msg);
			    	    rc = zmq_recvmsg(mqe.socket, &msg, ZMQ_DONTWAIT);
						zmq_msg_close(&msg);
					} while(rc == 0); //TODO: refactor this so there is one loop.

					mqe.lastHeartbeatTime = getCurrentTime();
				}
			}
			zmq_msg_close(&msg);

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

    	    if(zmq_recvmsg(hbSocket, &msg, ZMQ_DONTWAIT) == 0)
    	    {
				//Receive Dataproduct ID
				int size = zmq_msg_size(&msg);
				char* s = (char*)malloc(size+1);
				memcpy(s, zmq_msg_data(&msg), size);
				s[size] = 0;
				dataproductID.assign(s, size);
				free(s);
				//zmq_msg_close(&msg); //Closed after the end of the if statement.

//				//Recieve port
//				zmq_msg_t msg1;
//				zmq_msg_init(&msg1);
//				zmq_recvmsg(hbSocket, &msg2, ZMQ_DONTWAIT); //These are guarunteed to not fail since this is a multi part message.
//				memcpy(&port, zmq_msg_data(&msg), 2);
//				zmq_msg_close(&msg1);

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
			    //TODO: Fill queue with how long (MAX) we'd like to wait for each message.
				mqe1.timetowaitBetweenHeartbeats = maxtime;
				mqe1.socket = zmq_socket(params->zmq_context, ZMQ_SUB);
				zmq_connect(mqe1.socket, "tcp://*:54541"); //TODO: GET ENDPOINT!!!

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

struct HBParams {
	void* zmq_context;
	int interval_in_microseconds;
	//unsigned short port;
};

void* Heartbeat(void* thread_context)
{
	HBParams* params = (HBParams*) thread_context;
	void* heartbeatSocket = zmq_socket(params->zmq_context, ZMQ_PUB);
    zmq_bind(heartbeatSocket, "tcp://*:54541"); //Arbitrary port

    //Setup Message
    string str = "Good";
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());

	while(true)
	{
		zmq_sendmsg(heartbeatSocket, &msg, 0); //TODO: is message reusable?
#ifdef WIN32
		Sleep(params->interval_in_microseconds);
#else
		struct timespec request;
		request.tv_sec = params->interval_in_microseconds / 1000000;
		request.tv_nsec = (params->interval_in_microseconds % 1000000)*1000; //Convert from Microseconds to Nanoseconds
		clock_nanosleep(CLOCK_REALTIME, 0, request, NULL);
#endif
	}

	//This will never be reached but should be done when the thread ends.
	zmq_msg_close(&msg);
	delete params;

	return NULL;
}

} //namespace gravity
