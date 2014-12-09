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
#ifdef _WIN32
#include <winSock2.h>
#include <WinBase.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

using namespace std;

namespace gravity
{

ServiceDirectoryUDPReceiver::~ServiceDirectoryUDPReceiver(){};


void ServiceDirectoryUDPReceiver::start()
{

	sdSocket = zmq_socket(context,ZMQ_SUB);
	zmq_connect(sdSocket,"inproc://service_directory_udp_receive");
	zmq_setsockopt(sdSocket,ZMQ_SUBSCRIBE,NULL,0);

	// Poll the gravity node
	zmq_pollitem_t pollItem;
	pollItem.socket = sdSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;

	// Start polling socket(s), blocking while we wait
	int rc = zmq_poll(&pollItem, 1, -1); // 0 --> return immediately, -1 --> blocks
	if (rc == -1)
	{
		Log::debug("Interrupted, exiting (rc = %d)", rc);
		// Interrupted
		return;
	}

	if (pollItem.revents & ZMQ_POLLIN)
	{
		string command = readStringMessage(sdSocket);

		if (command == "recieve")
		{
			receiveReceiverParameters();
		}
	}

	while(true)
	{

	}




    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */
    unsigned int broadcastPort;     /* Port */
    char recvString[MAXRECVSTRING+1]; /* Buffer for received string */
    int recvStringLen;                /* Length of received string */


    broadcastPort = 0;    /* First arg: broadcast port */

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        std::cout << "socket() failed";
	return;
    }

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
    {
        std::cout << "bind() failed";
	return;
    }

	bool loop = true;
    while(loop)
    {

	    /* Receive a single datagram from the server */
	    if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
	    {
		std::cout << "recvfrom() failed";
		return;
	    }

	    recvString[recvStringLen] = '\0';
	    printf("Received: %s\n", recvString);    /* Print the received string */
	    
    }
}

void ServiceDirectoryUDPReceiver::receiveReceiverParameters()
{
	ourDomain = readStringMessage(sdSocket);

	//receive port
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(sdSocket,&msg,ZMQ_DONTWAIT);
	memcpy(&port,zmq_msg_data(&msg),sizeof(unsigned int));
	zmq_msg_close(&msg);

	//receive number of valid domains
	unsigned int numDomains = 0;
	zmq_msg_t msg2;
	zmq_msg_init(&msg2);
	zmq_recvmsg(sdSocket,&msg2,ZMQ_DONTWAIT);
	memcpy(&numDomains,zmq_msg_data(&msg2),sizeof(unsigned int));
	zmq_msg_close(&msg2);

	zmq_msg_t msg3;
	zmq_msg_init(&msg3);
	zmq_recvmsg(sdSocket,&msg3,ZMQ_DONTWAIT);
	int size = zmq_msg_size(&msg3);
	char* domains = (char*) malloc(size+1);
	memcpy(domains,zmq_msg_data(&msg3),size);
	domains[size]=0;
	string domainsString;
	domainsString.assign(domains);
	free(domains);

	parseValidDomains(domainsString,numDomains);

}

void ServiceDirectoryUDPReceiver::parseValidDomains(string domainString,unsigned int numDomains)
{
	numValidDomains = numDomains;

	validDomains = (string*) malloc(sizeof(string)*numDomains);
	//char** validDomains =(char**) malloc(numDomains*sizeof(char*));

	int start=0;
	int end = domainString.find(",",start);

	for(int i = 0; i < numDomains; i++)
	{
		if(end == string::npos)
		{
			end = domainString.length;
		}
		string sub = domainString.substr(start,end-start);
		validDomains[i]=sub;

		start=end+1;
		int end = domainString.find(",",start);
	}
}

int ServiceDirectoryUDPReceiver::initReceiveSocket()
{

}

}/*namespace gravity*/