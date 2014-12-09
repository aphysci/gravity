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
 * ServiceDirectoryUDPBroadcaster.h
 *
 *  Created on: Dec 3, 2014
 *      Author: Joseph Hankin
 */

#ifndef SERVICEDIRECTORYUDPBROADCASTER__H__
#define SERVICEDIRECTORYUDPBROADCASTER__H__

#include <string>
#include <zmq.h>

namespace gravity{


class ServiceDirectoryUDPBroadcaster
{
private:
	void* context;
	std::string domainName;
	std::string url;
	unsigned int port;
	unsigned int broadcastRate;
	int broadcastSocket;
	struct sockaddr* destAddress;
	static bool loop;
	void* sdSocket;

	void receiveBroadcastParameters();
	int initBroadcastSocket();


public:
	ServiceDirectoryUDPBroadcaster(void* __context__):
		context(__context__){}

	virtual ~ServiceDirectoryUDPBroadcaster();

	void start();

	void setDomainName(std::string domain){domainName=domain;};
	void setBroadcastRate(u_int rate){broadcastRate=rate;};


};
}/* namespace gravity */
#endif //SERVICEDIRECTORYUDPBROADCASTER__H__