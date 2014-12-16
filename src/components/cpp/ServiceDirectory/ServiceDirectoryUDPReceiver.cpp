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

/*
 * ServiceDirectoryUDPReceiver.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: Joseph Hankin
 */

#include "ServiceDirectoryUDPReceiver.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <errno.h>
#include<set>
#ifdef _WIN32
#include <winSock2.h>
#include <WinBase.h>
#include <Windows.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include "protobuf/ServiceDirectoryBroadcastPB.pb.h"

using namespace std;

#ifdef _WIN32

static int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

static struct timeval addTime(struct timeval *t1, int sec)
{
	struct timeval newTime;
	newTime.tv_sec=t1->tv_sec+sec;
	newTime.tv_usec= t1->tv_usec;

	return newTime;
}
static int timevalcmp(struct timeval* t1, struct timeval* t2)
{
	if (t1->tv_sec ==t2->tv_sec)
	{
		if(t1->tv_usec == t2->tv_usec)
		{
			return 0;
		}
		else if(t1->tv_usec < t2->tv_usec)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if(t1->tv_sec < t2->tv_sec)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

static struct timeval subtractTime(struct timeval *t1, struct timeval *t2)
{
	struct timeval newTime;
	newTime.tv_sec=0;
	newTime.tv_usec=0;
	//make sure t1 is greater than t2
	if(timevalcmp(t1,t2) <=0)
	{
		return newTime;
	}

	newTime.tv_sec=t1->tv_sec - t2->tv_sec;
	long sub = t1->tv_usec;
	if(t2->tv_usec > t1->tv_usec)
	{
		sub = 1000000;
	}
	newTime.tv_usec=sub-t2->tv_usec;

	return newTime;
}

static unsigned int timevalToMilliSeconds(struct timeval *tv)
{
/*	if(tv->tv_sec == 0 && tv->tv_usec == 0)
	{
		return -1;
	}
*/
	return (unsigned int) (tv->tv_usec/1000) + (tv->tv_sec*1000);
}

static void printByteBuffer(char* buffer, int len)
{
	for(int i = 0; i < len; i++)
	{
		printf("%02X ",*(buffer+i));
	}
	printf("\n");
}


namespace gravity
{

map<string,unsigned int> receivedCountMap;
map<string,unsigned int> broadcastRateMap;
map<string,struct timeval> expectedMsgTimeMap;
set<string> connectedDomainSet;

ServiceDirectoryUDPReceiver::~ServiceDirectoryUDPReceiver()
{
	#ifdef _WIN32
	closesocket(receiveSocket);
	#else
	close(receiveSocket);
	#endif
	zmq_close(sdSocket);
};


void ServiceDirectoryUDPReceiver::start()
{

	sdSocket = zmq_socket(context,ZMQ_REP);
	zmq_connect(sdSocket,"inproc://service_directory_udp_receive");
	zmq_setsockopt(sdSocket,ZMQ_SUBSCRIBE,NULL,0);

	void* domainSocket = zmq_socket(context,ZMQ_PUB);
	zmq_connect(domainSocket,"inproc://service_directory_domain_socket");
	

	// Poll the service directory node
	zmq_pollitem_t pollItem;
	pollItem.socket = sdSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;

	bool waiting = true;
	// Wait for command to start reciever
	// Start polling socket(s), blocking while we wait
	while(waiting)
	{

		int rc = zmq_poll(&pollItem, 1, -1); // 0 --> return immediately, -1 --> blocks
		if (rc == -1)
		{
			Log::fatal("Interrupted, exiting (rc = %d)", rc);
			// Interrupted
			return;
		}

		if (pollItem.revents & ZMQ_POLLIN)
		{
			string command = readStringMessage(sdSocket);

			if (command == "receive")
			{
				receiveReceiverParameters();
				if(initReceiveSocket()<0)
				{
					Log::fatal("UDP Receiver init error");
				}
				waiting = false;
			}
		}
	}

	char recvString[MAXRECVSTRING+1]; /* Buffer for received string */
    int recvStringLen;                /* Length of received string */
	ServiceDirectoryBroadcastPB broadcastPB;

	struct timeval currTime;
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;

	//set socket to block forever initially
#ifdef _WIN32
	unsigned int timeout_int = timevalToMilliSeconds(&timeout);
	setsockopt(receiveSocket,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout_int,sizeof(unsigned int));
#else
	//set socket to block forever initially
	setsockopt(receiveSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
#endif



	while(true)
	{
		memset(recvString,0,MAXRECVSTRING+1);

		/* Receive a broadcast message or timeout */
	    recvStringLen = recvfrom(receiveSocket, recvString, MAXRECVSTRING, 0, NULL, 0);
		
		//get the current time
		gettimeofday(&currTime,NULL);

	
		if(recvStringLen == 0)
		{
			// Don't know why this would happen, just ignore this case.
			continue;
		}

		//check for socket error
		else if(recvStringLen < 0)
		{
			if (!(errno == EAGAIN || errno == EWOULDBLOCK))
			{
				Log::fatal("recv() error, errno: %d", errno);
				break;
			}
		}
		else //we received a message
		{

			broadcastPB.ParseFromArray(recvString,recvStringLen);
			//printByteBuffer(recvString,recvStringLen);

			
			//ignore messages from our domain name or invalid domains
			if((ourDomain.compare(broadcastPB.domain())!=0) && isValidDomain(broadcastPB.domain()))
			{
				Log::debug("Received UDP Broadcast Message for Domain: %s",broadcastPB.domain().c_str());

				//if first time seeing domain
				if(receivedCountMap.find(broadcastPB.domain())==receivedCountMap.end())
				{
					receivedCountMap[broadcastPB.domain()]=1;
					broadcastRateMap[broadcastPB.domain()]=broadcastPB.rate();
				}
				else
				{
					unsigned int count = receivedCountMap.at(broadcastPB.domain());
					count++;

					//if enough broadcasts have been seen
					if(count >= MAX_RECEIVE_COUNT)
					{
						//cap the count
						count = MAX_RECEIVE_COUNT;

						//check whether we have already connected with this domain
						if(connectedDomainSet.find(broadcastPB.domain()) == connectedDomainSet.end())
						{
							// add domain to the connected list
							connectedDomainSet.insert(broadcastPB.domain());
							// Inform SD of new connection
							sendStringMessage(domainSocket,"Add",ZMQ_SNDMORE);
							sendStringMessage(domainSocket,broadcastPB.domain(),ZMQ_SNDMORE);
							sendStringMessage(domainSocket,broadcastPB.url(),ZMQ_DONTWAIT);

						}
					}
					//update count for domain
					receivedCountMap[broadcastPB.domain()]=count; 
				}
				//insert new expected time for domain
				expectedMsgTimeMap[broadcastPB.domain()]=addTime(&currTime,broadcastPB.rate());
			}
		}

		timeout.tv_sec=0;
		timeout.tv_usec=0;
		
		set<string>removeSet;
		for(map<string,struct timeval>::iterator iter=expectedMsgTimeMap.begin();iter != expectedMsgTimeMap.end();++iter)
		{
			bool removed = false;
			//check if we missed a message
			if(timevalcmp(&(iter->second),&currTime) <= 0)
			{
				unsigned int count = receivedCountMap.at(iter->first);
				count--;
				//if we have missed enough messages
				if(count <=0)
				{
					// inform Service Directory to remove domain
					sendStringMessage(domainSocket,"Remove",ZMQ_SNDMORE);
					sendStringMessage(domainSocket,iter->first,ZMQ_DONTWAIT);

					// remove domain from data sets
					receivedCountMap.erase(iter->first);
					broadcastRateMap.erase(iter->first);
					connectedDomainSet.erase(iter->first);
					removeSet.insert(iter->first);
					removed = true;
				}
				else
				{
					//set new expected message time for missed doamin
					expectedMsgTimeMap[iter->first]=addTime(&(iter->second),broadcastRateMap.at(iter->first));
					receivedCountMap[iter->first]=count;
				}
			}

			if(!removed)
			{
				struct timeval* nextTime = &(iter->second);

				struct timeval timeToWait = subtractTime(nextTime,&currTime);
				struct timeval zeroTime;
				zeroTime.tv_sec=0;
				zeroTime.tv_usec=0;
				//if the time to wait is less then the current timeout
				if((timevalcmp(&zeroTime,&timeout)==0) || (timevalcmp(&timeToWait,&timeout) < 0))
				{
					timeout=timeToWait;
				}
			}
		}

		//remove any leftover domains from the expected message time map
		for(set<string>::iterator iter = removeSet.begin(); iter != removeSet.end(); ++iter)
		{
			expectedMsgTimeMap.erase(*iter);
		}
#ifdef _WIN32
		//select(receiveSocket,&fds,NULL,NULL,&timeout);
		timeout_int = timevalToMilliSeconds(&timeout);
		setsockopt(receiveSocket,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout_int,sizeof(unsigned int));
#else
		//set socket to block until the timeout
		setsockopt(receiveSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
#endif
	}

	#ifdef _WIN32
	closesocket(receiveSocket);
	#else
	close(receiveSocket);
	#endif
	zmq_close(sdSocket);
}

void ServiceDirectoryUDPReceiver::receiveReceiverParameters()
{
	ourDomain = readStringMessage(sdSocket);

	//receive port
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(sdSocket,&msg,ZMQ_DONTWAIT);
	std::memcpy(&port,zmq_msg_data(&msg),sizeof(unsigned int));
	zmq_msg_close(&msg);

	//receive number of valid domains
	unsigned int numDomains = 0;
	zmq_msg_t msg2;
	zmq_msg_init(&msg2);
	zmq_recvmsg(sdSocket,&msg2,ZMQ_DONTWAIT);
	std::memcpy(&numDomains,zmq_msg_data(&msg2),sizeof(unsigned int));
	zmq_msg_close(&msg2);

	//recieve csv list of domains
	zmq_msg_t msg3;
	zmq_msg_init(&msg3);
	zmq_recvmsg(sdSocket,&msg3,ZMQ_DONTWAIT);
	int size = zmq_msg_size(&msg3);
	char* domains = (char*) malloc(size+1);
	std::memcpy(domains,zmq_msg_data(&msg3),size);
	domains[size]=0;
	string domainsString;
	domainsString.assign(domains);
	std::free(domains);

	parseValidDomains(domainsString,numDomains);

	sendStringMessage(sdSocket,"ACK",ZMQ_DONTWAIT);

}

int ServiceDirectoryUDPReceiver::initReceiveSocket()
{                        
	/* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */

    /* Create a best-effort datagram socket using UDP */
    if ((receiveSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
		Log::fatal("Receiver: socket() failed");
		return receiveSocket;
    }

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(port);      /* Broadcast port */

    /* Bind to the broadcast port */
	int rc = 0;
    if ((rc == bind(receiveSocket, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr))) < 0)
    {
		Log::fatal("Receiver: bind() failed");
		return rc;
    }

	return receiveSocket;
}


void ServiceDirectoryUDPReceiver::parseValidDomains(string domainString,unsigned int numDomains)
{
	int start=0;
	int end = domainString.find(",",start);

	for(int i = 0; i < numDomains; i++)
	{
		if(end == string::npos)
		{
			end = domainString.length();
		}
		string sub = domainString.substr(start,end-start);
		validDomains.push_back(sub);

		start=end+1;
		int end = domainString.find(",",start);
	}
}

bool ServiceDirectoryUDPReceiver::isValidDomain(string domain)
{
	for(vector<string>::iterator iter = validDomains.begin();iter!=validDomains.end();++iter)
	{
		if((domain.compare(*iter)==0)||((*iter).compare("*")==0))
		{
			return true;
		}
	}

	return false;
}

}/*namespace gravity*/