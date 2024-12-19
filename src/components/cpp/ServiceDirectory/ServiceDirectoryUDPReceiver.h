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
 * ServiceDirectoryUDPReceiver.h
 *
 *  Created on: Dec 3, 2014
 *      Author: Joseph Hankin
 */

#ifndef SERVICEDIRECTORYUDPRECEIVER__H__
#define SERVICEDIRECTORYUDPRECEIVER__H__

#include <vector>
#include <string>
#include <zmq.h>
#include "spdlog/spdlog.h"

namespace gravity
{

class ServiceDirectoryUDPReceiver
{
private:
	static const int MAX_RECEIVE_COUNT = 3;

	void* context;
	void* sdSocket;
	std::string ourDomain;
	std::string ourUrl;
	unsigned int port;
	int receiveSocket;

	std::vector<std::string> validDomains;

	void receiveReceiverParameters();
	int initReceiveSocket();
	void parseValidDomains(std::string domainString,unsigned int num);
	bool isValidDomain(std::string domain);


public:
	ServiceDirectoryUDPReceiver(void* context);

	virtual ~ServiceDirectoryUDPReceiver();

	void start();

};
} /*namespace gravity*/
#endif //SERVICEDIRECTORYUDPRECEIVER__H__