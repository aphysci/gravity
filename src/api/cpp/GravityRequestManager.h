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
 * GravityRequestManager.h
 *
 *  Created on: Aug 28, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYREQUESTMANAGER_H_
#define GRAVITYREQUESTMANAGER_H_

#include <zmq.h>
#include <vector>
#include "GravityNode.h"

namespace gravity
{

typedef struct RequestDetails
{
	std::string serviceID;
	std::string requestID;
	GravityRequestor* requestor;
} RequestDetails;

class GravityRequestManager
{
private:
	void* context;
	void* gravityNodeSocket;
	void* gravityResponseSocket;
	std::map<void*,std::tr1::shared_ptr<RequestDetails> > requestMap;
	std::vector<zmq_pollitem_t> pollItems;
	std::map<std::string,void*> futureResponseUrlToSocketMap;
	void processRequest();
	void createFutureResponse();
	void sendFutureResponse();
	void ready();
public:
	/**
	 * Constructor GravityRequestManager
	 * \param context The zmq context in which the socket will be established with the service provider
	 */
	GravityRequestManager(void* context);

	/**
	 * Default destructor
	 */
	virtual ~GravityRequestManager();

	/**
	 * Starts the GravityRequestManager which will manage the request/response communication with
	 * the service provider
	 */
	void start();
};

} /* namespace gravity */
#endif /* GRAVITYREQUESTMANAGER_H_ */
