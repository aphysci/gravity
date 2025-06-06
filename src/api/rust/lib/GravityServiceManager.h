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
 * GravityServiceManager.h
 *
 *  Created on: Aug 30, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSERVICEMANAGER_H_
#define GRAVITYSERVICEMANAGER_H_

#include <zmq.h>
#include <vector>
#include "GravityNode.h"

namespace spdlog
{
class logger;
}

#define SERVICE_MGR_URL "inproc://gravity_service_manager"

namespace gravity
{

typedef struct ServiceDetails
{
    std::string serviceID;
    std::string url;
    zmq_pollitem_t pollItem;
    GravityServiceProvider* server;
} ServiceDetails;

class GravityServiceManager
{
private:
    void* context;
    void* gravityNodeSocket;
    std::map<void*, std::shared_ptr<ServiceDetails> > serviceMapBySocket;
    std::map<std::string, std::shared_ptr<ServiceDetails> > serviceMapByServiceID;
    std::map<std::string, uint32_t> serviceRegistrationTimeMap;
    std::vector<zmq_pollitem_t> pollItems;
    void addService();
    void removeService();
    void ready();
    std::shared_ptr<spdlog::logger> logger;

public:
    /**
	 * Constructor GravityServiceManager
	 * \param context The zmq context used for connections
	 */
    GravityServiceManager(void* context);

    /**
	 * Default destructor
	 */
    virtual ~GravityServiceManager();

    /**
	 * Starts the GravityServiceManager which will manage the communication with the service client
	 */
    void start();
};

} /* namespace gravity */
#endif /* GRAVITYSERVICEMANAGER_H_ */
