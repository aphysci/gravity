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
 * ServiceDirectoryUDPBroadcaster.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: Joseph Hankin
 */

#include "ServiceDirectoryUDPBroadcaster.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
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
ServiceDirectoryUDPBroadcaster::~ServiceDirectoryUDPBroadcaster(){};


void ServiceDirectoryUDPBroadcaster::start()
{
	sdSocket = zmq_socket(context,ZMQ_SUB);
	zmq_connect(sdSocket,"inproc://service_directory_udp_broadcast");
	zmq_setsockopt(sdSocket,ZMQ_SUBSCRIBE,NULL,0);

	// Poll the gravity node
	zmq_pollitem_t pollItem;
	pollItem.socket = sdSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;

	int block = -1;

	bool broadcast = false;
	while(true)
	{
		// Start polling socket(s), blocking while we wait
		int rc = zmq_poll(&pollItem, 1, block); // 0 --> return immediately, -1 --> blocks
		if (rc == -1)
		{
			Log::debug("Interrupted, exiting (rc = %d)", rc);
			// Interrupted
			break;
		}

		if (pollItem.revents & ZMQ_POLLIN)
		{
			string command = readStringMessage(sdSocket);

			if (command == "broadcast")
			{
				block = 0;
				//read broadcast parameters
				receiveBroadcastParameters();
				initBroadcastSocket();
				broadcast = true;
			}
			else if (command == "stop")
			{
				#ifdef _WIN32
				closesocket(broadcastSocket);
				#else
				close(broadcastSocket);
				#endif
				block = -1;
				broadcast = false;
			}
			else if(command =="kill")
			{
				break;
			}
		}
		if(broadcast)
		{
			//broadcast
			int bytesSent = sendto(broadcastSocket,domainName.c_str(),strlen(domainName.c_str()),0,destAddress,sizeof(struct sockaddr_in));
			if (bytesSent < 0)
			{
				//exit
				return;
			}
			#ifdef _WIN32
			Sleep(broadcastRate);
			#else
			usleep(broadcastRate*1000); //Maybe replace this guy with clock_nanosleep???
			#endif
		}
	}

	//close sockets
	zmq_close(sdSocket);

}

void ServiceDirectoryUDPBroadcaster::receiveBroadcastParameters()
{
	domainName = readStringMessage(sdSocket);
	url = readStringMessage(sdSocket);

	//receive port
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(sdSocket,&msg,ZMQ_DONTWAIT);
	memcpy(&port,zmq_msg_data(&msg),sizeof(unsigned int));
	zmq_msg_close(&msg);

	//receive broadcast rate
	zmq_msg_t msg2;
	zmq_msg_init(&msg2);
	zmq_recvmsg(sdSocket,&msg2,ZMQ_DONTWAIT);
	memcpy(&broadcastRate,zmq_msg_data(&msg2),sizeof(unsigned int));
	zmq_msg_close(&msg2);

}

int ServiceDirectoryUDPBroadcaster::initBroadcastSocket()
{
	struct sockaddr_storage destStorage;
	std::memset(&destStorage,0,sizeof(destStorage));

	struct sockaddr_in *destAddr = (struct sockaddr_in*) &destStorage;
	destAddr->sin_family=AF_INET;
	destAddr->sin_port=htons(port);
	destAddr->sin_addr.s_addr = inet_addr("255.255.255.255");


	destAddress = (struct sockaddr*) &destStorage;

	broadcastSocket = socket(destAddress->sa_family,SOCK_DGRAM,IPPROTO_UDP);

	if(broadcastSocket < 0)
	{
		return -1;
	}

	int broadcastPerm=1;
	if(setsockopt(broadcastSocket,SOL_SOCKET,SO_BROADCAST,(char *)&broadcastPerm,sizeof(broadcastPerm)) < 0)
	{
		return -1;
	}

	return broadcastSocket;
}
}/*namespace gravity*/