#include <iostream>
#include <queue>
#include <list>

#include "GravityHeartbeatListener.h"

namespace gravity
{

#ifdef FINISHED_WITH_HEARTBEAT

using namespace std;

struct ExpectedMessasgeQueueElement {
	uint64_t expectedTime; //Absolute (Maximum amount we can wait).
	uint64_t lastHeartbeatTime; //Absolute
	uint64_t timetowaitBetweenHeartbeats; //In Microseconds
	std::string dataproductID;
	void* socket;
};

bool operator< (const ExpectedMessasgeQueueElement &a, ExpectedMessasgeQueueElement &b)
{
	return a.expectedTime < b.expectedTime;
}

void* startHeartbeatListener(void* context)
{
    void* hbSocket = zmq_socket(context, ZMQ_REP);
    zmq_bind(hbSocket, "inproc://heartbeat_listener");

    list<ExpectedMessasgeQueueElement> queueElements; //This is so we can reuse these guys.
    priority_queue<ExpectedMessasgeQueueElement&> messageTimes;
    //TODO: Fill queue with how long (MAX) we'd like to wait for each message.

    while(true)
    {
    	if(!messageTimes.empty()) //This should never happen because we should start with at least 1 heartbeat to monitor.
    	{
    		zmq_msg_t msg;
    	    zmq_msg_init(&msg);

			ExpectedMessasgeQueueElement& mqe = messageTimes.top();
			wait(mqe.expectedTime - getCurrentTime());

			if(zmq_recvmsg(mqe.socket, &msg, ZMQ_NOBLOCK) != 0)
				if(zmq_errno() == EAGAIN)
					gn.findHeartbeatListener(mqe.dataproductID)->MissedHeartbeat(mqe.dataproductID, mqe.lastHeartbeatTime - getCurrentTime(), "Missed");
				else
					gn.findHeartbeatListener(mqe.dataproductID)->MissedHeartbeat(mqe.dataproductID, mqe.lastHeartbeatTime - getCurrentTime(), "SocketError");
			else
			{
				if(zmq_msg_size(&msg) != 4 && strncmp(zmq_msg_data(&msg), "Good", 4) != 0)
					gn.findHeartbeatListener(mqe.dataproductID)->MissedHeartbeat(mqe.dataproductID, mqe.lastHeartbeatTime - getCurrentTime(), string(zmq_msg_data(&msg), zmq_msg_size(&msg)));
				else
				{
					//Receive any other extrainious heartbeats we may have missed.  //TODO: check status.
					int rc;
					do
					{
			    		zmq_msg_t msg;
			    	    zmq_msg_init(&msg);
			    	    rc = zmq_recvmsg(socket, &msg, ZMQ_NOBLOCK);
						zmq_msg_close(&msg);
					} while(rc == 0); //TODO: refactor this so there is one loop.

					mqe.lastHeartbeatTime = getCurrentTime();
				}

				zmq_msg_close(&msg);
			}

			messageTimes.pop();
			mqe.expectedTime = getCurrentTime() + timetowaitBetweenHeartbeats;
			messageTimes.push(mqe);
    	}

        //Allow Gravity to add Heartbeat listeners.
    	{
    		zmq_msg_t msg;
    	    zmq_msg_init(&msg);
    	    std::string dataproductID;
    	    unsigned short port;
    	    uint64_t maxtime;

    	    if(zmq_recvmsg(hbSocket, &msg, ZMQ_NOBLOCK) == 0)
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
//				zmq_recvmsg(hbSocket, &msg, ZMQ_NOBLOCK); //These are guarunteed to not fail since this is a multi part message.
//				memcpy(&port, zmq_msg_data(&msg), 2);
//				zmq_msg_close(&msg1);

				//Receive maxtime.
				zmq_msg_t msg2;
				zmq_msg_init(&msg2);
				zmq_recvmsg(hbSocket, &msg, ZMQ_NOBLOCK);
				memcpy(&maxtime, zmq_msg_data(&msg), 8);
				zmq_msg_close(&msg2);

				ExpectedMessasgeQueueElement mqe1;
				mqe1.dataproductID = dataproductID;
				mqe1.expectedTime = getCurrentTime() + maxtime;
				mqe1.lastHeartbeatTime = 0;
				mqe1.timetowaitBetweenHeartbeats = maxtime;
				mqe1.socket = zmq_socket(context, ZMQ_SUB);
				zmq_bind(mqe1.socket, "tcp://*:54541"); //Arbitrary port

				queueElements.push_back(mqe1);
				messageTimes.push(queueElements.back());
    	    }

	    	zmq_msg_close(&msg);

    	}//Gravity

    }//while(true)

    zmq_msg_close(&msg);
}


void* startHeartbeat(void* context)
{
	void* heartbeatSocket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(initSocket, "tcp://*:54541"); //Arbitrary port

    //Setup Message
    string str = "Good";
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());

	while(true)
	{
		zmq_sendmsg(socket, &msg, flags);
		sleep(interval);
	}

	//This will never be reached but should be done when the thread ends.
	zmq_msg_close(&msg);
}

#else //Not finished with Heartbeat.  This should keep things from breaking.

void* startHeartbeat(void* context)
{
	while(true)
		;
}

void* startHeartbeatListener(void* context)
{
	while(true)
		;
}

#endif

} //namespace gravity
