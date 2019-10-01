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
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <errno.h>
#include<set>
#ifdef _WIN32
#include <winSock2.h>
#include <WS2tcpip.h>
#include <winsock.h>
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


namespace gravity
{

map<string,unsigned int> receivedCountMap;
map<string,unsigned int> broadcastRateMap;
map<string,struct timeval> expectedMsgTimeMap;
map<string, int64_t> connectedDomainMap;

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

			//Log a warning if we received the same domain from a different url
			if((ourDomain.compare(broadcastPB.domain())==0) && ourUrl.compare(broadcastPB.url())!=0)
			{
				Log::warning("Duplicate Domain: %s, %s",ourDomain.c_str(),broadcastPB.url().c_str());
			}
			
			//ignore messages from our domain name or invalid domains
			if((ourDomain.compare(broadcastPB.domain())!=0) && isValidDomain(broadcastPB.domain()))
			{
				//Log::trace("Received UDP Broadcast Message for Domain: %s",broadcastPB.domain().c_str());

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
					if(count >= (unsigned int) MAX_RECEIVE_COUNT)
					{
						//cap the count
						count = MAX_RECEIVE_COUNT;

						//check whether we have already connected with this domain
						if (connectedDomainMap.find(broadcastPB.domain()) == connectedDomainMap.end())
						{
							// add domain to the connected list
							connectedDomainMap[broadcastPB.domain()] = broadcastPB.starttime();
							
							// Inform SD of new connection
							Log::trace("Sending domain Add command to synchronizer thread");
							sendStringMessage(domainSocket,"Add",ZMQ_SNDMORE);
							sendStringMessage(domainSocket,broadcastPB.domain(),ZMQ_SNDMORE);
							sendStringMessage(domainSocket,broadcastPB.url(),ZMQ_DONTWAIT);
						}
						else if (connectedDomainMap[broadcastPB.domain()] != broadcastPB.starttime())
						{
							// update domain time
							connectedDomainMap[broadcastPB.domain()] = broadcastPB.starttime();

							// Inform SD of updated connection
							Log::trace("Sending domain Update command to synchronizer thread");
							sendStringMessage(domainSocket, "Update", ZMQ_SNDMORE);
							sendStringMessage(domainSocket, broadcastPB.domain(), ZMQ_SNDMORE);
							sendStringMessage(domainSocket, broadcastPB.url(), ZMQ_DONTWAIT);
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
				int count = receivedCountMap.at(iter->first);
				count--;
				//if we have missed enough messages
				if(count <=0)
				{
					//check whether we have already connected with this domain
					if (connectedDomainMap.find(iter->first) != connectedDomainMap.end())
					{
						// inform Service Directory to remove domain
						sendStringMessage(domainSocket,"Remove",ZMQ_SNDMORE);
						sendStringMessage(domainSocket,iter->first,ZMQ_DONTWAIT);
						connectedDomainMap.erase(iter->first);
					}

					// remove domain from data sets
					receivedCountMap.erase(iter->first);
					broadcastRateMap.erase(iter->first);
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
	ourUrl = readStringMessage(sdSocket);

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

	//set socket to be re-usable. Must be set for all other listeners for this port
	int one = 1;
	setsockopt(receiveSocket,SOL_SOCKET,SO_REUSEADDR,(const char*)&one,sizeof(one));

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(port);      /* Broadcast port */                                                                                                                                                     

    /* Bind to the broadcast port */
#ifdef _WIN32
	int rc = bind((SOCKET)receiveSocket, (const struct sockaddr *) &broadcastAddr, (int)sizeof(broadcastAddr));
#else
	int rc = bind(receiveSocket, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
#endif
	if (rc < 0)
    {
		Log::fatal("Receiver: bind() failed");
		return rc;
    }

	return receiveSocket;
}


void ServiceDirectoryUDPReceiver::parseValidDomains(string domainString,unsigned int numDomains)
{
	int start=0;
	unsigned int end = domainString.find(",",start);

	for(unsigned int i = 0; i < numDomains; i++)
	{
		if(end == string::npos)
		{
			end = domainString.length();
		}
		string sub = domainString.substr(start,end-start);
		validDomains.push_back(sub);

		start=end+1;
		end = domainString.find(",",start);
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
