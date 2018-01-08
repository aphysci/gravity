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
#include "GravityPublishManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"

namespace gravity
{

using namespace std;
using namespace std::tr1;

std::list<ExpectedMessageQueueElement> Heartbeat::queueElements; //This is so we can reuse these guys.
std::priority_queue<ExpectedMessageQueueElement*, vector<ExpectedMessageQueueElement*>, EMQComparator> Heartbeat::messageTimes;
std::map<std::string, GravityHeartbeatListener*> Heartbeat::listener;

Semaphore Heartbeat::lock;
std::set<std::string> Heartbeat::filledHeartbeats;


void Heartbeat::subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts)
{
	lock.Lock();
	for(size_t i = 0; i < dataProducts.size(); i++)
		filledHeartbeats.insert(dataProducts[i]->getDataProductID());
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
			ExpectedMessageQueueElement& mqe = *messageTimes.top();

			if (mqe.expectedTime > getCurrentTime())
			{
		 		gravity::sleep(100);
			}
			else
			{
				lock.Lock();

				std::set<std::string>::iterator i = filledHeartbeats.find(mqe.dataproductID);
				bool gotHeartbeat = (i != filledHeartbeats.end());
				if(gotHeartbeat)
					filledHeartbeats.erase(i);
				lock.Unlock();
				if(listener.find(mqe.dataproductID)!=listener.end())
				{
					//chop off "_GravityHeartbeat" before sending DataProductID to listeners
					std::string componentId = mqe.dataproductID.substr(0,mqe.dataproductID.rfind("_GravityHeartbeat"));

					if(!gotHeartbeat)
					{
						int64_t diff = mqe.lastHeartbeatTime == 0 ? -1l : getCurrentTime() - mqe.lastHeartbeatTime;
						listener[mqe.dataproductID]->MissedHeartbeat(componentId, diff, mqe.timetowaitBetweenHeartbeats);
					}
					else
					{
						listener[mqe.dataproductID]->ReceivedHeartbeat(componentId, mqe.timetowaitBetweenHeartbeats);
						mqe.lastHeartbeatTime = getCurrentTime();
					}
				}
				messageTimes.pop();
				mqe.expectedTime = getCurrentTime() + mqe.timetowaitBetweenHeartbeats; //(Maybe should be lastHeartbeatTime + timetowaitBetweenHeartbeats, but current version allows for drift etc.)
				messageTimes.push(&mqe);
			}

    	}//Process Messages

        //Allow Gravity to add Heartbeat listeners.
    	{
    		zmq_msg_t msg;
    	    zmq_msg_init(&msg);
    	    std::string dataproductID;
    	    //unsigned short port;
    	    int64_t maxtime;

    	    if(zmq_recvmsg(hbSocket, &msg, ZMQ_DONTWAIT) != -1)
    	    {
				std::string command;
				int size = zmq_msg_size(&msg);
				char* c = (char*)malloc(size+1);
				memcpy(c, zmq_msg_data(&msg), size);
				c[size] = 0;
				command.assign(c,size);
				free(c);

				if (command == "register")
				{
					zmq_msg_t msg2;
					zmq_msg_init(&msg2);
					//Receive Dataproduct ID
					zmq_recvmsg(hbSocket,&msg2,ZMQ_DONTWAIT);
					size = zmq_msg_size(&msg2);
					char* s = (char*)malloc(size+1);
					memcpy(s, zmq_msg_data(&msg2), size);
					s[size] = 0;
					dataproductID.assign(s, size);
					free(s);
					zmq_msg_close(&msg2);

					//Receive address of listener
					zmq_msg_t msg3;
					zmq_msg_init(&msg3);
					intptr_t p;
					zmq_recvmsg(hbSocket, &msg3, ZMQ_DONTWAIT); //These are guarunteed to not fail since this is a multi part message.
					memcpy(&p, zmq_msg_data(&msg3), sizeof(intptr_t));
					zmq_msg_close(&msg3);

					listener[dataproductID] = (GravityHeartbeatListener*) p;

					//Receive maxtime.
					zmq_msg_t msg4;
					zmq_msg_init(&msg4);
					zmq_recvmsg(hbSocket, &msg4, ZMQ_DONTWAIT);
					memcpy(&maxtime, zmq_msg_data(&msg4), 8);
					zmq_msg_close(&msg4);

					ExpectedMessageQueueElement mqe1;
					mqe1.dataproductID = dataproductID;
					mqe1.expectedTime = getCurrentTime() + maxtime;
					mqe1.lastHeartbeatTime = 0;
					mqe1.timetowaitBetweenHeartbeats = maxtime;

					//We should already be subscribed to the Heartbeats.

					queueElements.push_back(mqe1);
					messageTimes.push(&queueElements.back()); 
				}
				else if (command == "unregister")
				{
					lock.Lock();

					zmq_msg_t msg2;
					zmq_msg_init(&msg2);
					//Receive Dataproduct ID
					zmq_recvmsg(hbSocket,&msg2,ZMQ_DONTWAIT);
					size = zmq_msg_size(&msg2);
					char* s = (char*)malloc(size+1);
					memcpy(s, zmq_msg_data(&msg2), size);
					s[size] = 0;
					dataproductID.assign(s, size);
					free(s);
					zmq_msg_close(&msg2);

					listener.erase(dataproductID);

					int queueSize = messageTimes.size();

					std::priority_queue<ExpectedMessageQueueElement*, vector<ExpectedMessageQueueElement*>, EMQComparator> tempQueue;

					//loop through whole queue to preserve order
					for(int i = 0; i <queueSize;i++)
					{
						ExpectedMessageQueueElement& entry = *messageTimes.top();
						messageTimes.pop();
						//add back all heartbeat entries that are not equal to dataProductID
						if (entry.dataproductID != dataproductID)
						{		
							tempQueue.push(&entry);
						}
					}

					messageTimes = tempQueue;
					
					lock.Unlock();
				}
	            else if (command == "kill")
	            {
	                break;
	            }

				// Send ACK
				sendStringMessage(hbSocket, "ACK", ZMQ_DONTWAIT);
    	    }
			else
			{
				int errnum = zmq_errno();
				if (errnum != EAGAIN)
				{
					Log::critical("Heartbeat Message Error: %s",zmq_strerror(errnum));
					break;
				}
			}

	    	zmq_msg_close(&msg);

    	}//Add Heartbeat listeners

    }//while(true)

    zmq_close(hbSocket);
    delete params;

    return NULL;
}

Semaphore Heartbeat::heartbeatLock;
bool Heartbeat::heartbeatRunning = false;

void Heartbeat::setHeartbeatRunning(bool running)
{
	Heartbeat::heartbeatLock.Lock();
	Heartbeat::heartbeatRunning = running;
	Heartbeat::heartbeatLock.Unlock();
}

bool Heartbeat::isHeartbeatRunning()
{
	bool running;
	Heartbeat::heartbeatLock.Lock();
	running = Heartbeat::heartbeatRunning;
	Heartbeat::heartbeatLock.Unlock();
	return running;
}

void* Heartbeat(void* thread_context)
{
	HBParams* params = (HBParams*) thread_context;
	GravityDataProduct gdp(params->componentID);
	gdp.setData((void*)"Good", 5);

	void *heartbeatSocket = zmq_socket(params->zmq_context,ZMQ_PUB);
	zmq_connect(heartbeatSocket,PUB_MGR_HB_URL);

	Heartbeat::setHeartbeatRunning(true);

	while(Heartbeat::isHeartbeatRunning())
	{
		// Publish heartbeat (via the GravityPublishManager)
		gdp.setTimestamp(getCurrentTime());
		Log::trace("%s: Publishing heartbeat", params->componentID.c_str());
		sendStringMessage(heartbeatSocket, "publish", ZMQ_SNDMORE);
		sendStringMessage(heartbeatSocket, gdp.getDataProductID(), ZMQ_SNDMORE);
		sendUint64Message(heartbeatSocket, gdp.getGravityTimestamp(), ZMQ_SNDMORE);
		sendStringMessage(heartbeatSocket, "", ZMQ_SNDMORE);
		zmq_msg_t msg;
		zmq_msg_init_size(&msg, gdp.getSize());
		gdp.serializeToArray(zmq_msg_data(&msg));
		zmq_sendmsg(heartbeatSocket, &msg, ZMQ_DONTWAIT);
		zmq_msg_close(&msg);
#ifdef WIN32
		Sleep(params->interval_in_microseconds/1000);
#else
		struct timespec request;
		request.tv_sec = params->interval_in_microseconds / 1000000;
		request.tv_nsec = (params->interval_in_microseconds % 1000000)*1000; //Convert from Microseconds to Nanoseconds
		clock_nanosleep(CLOCK_REALTIME, 0, &request, NULL);
#endif
	}

    zmq_close(heartbeatSocket);
	// Clean up
	delete params;

	return NULL;
}

} //namespace gravity
