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
 * ServiceDirectory.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#ifndef SERVICEDIRECTORY_H_
#define SERVICEDIRECTORY_H_

#include <string>
#include <list>
#include <map>
#include <set>
#include "GravityDataProduct.h"
#include "GravityNode.h"

using namespace std;

namespace gravity
{

class ServiceDirectory : GravityServiceProvider
{
private:
	static const int DEFAULT_BROADCAST_RATE_SEC  = 10;
	static const int DEFAULT_BROADCAST_PORT  = 5678;


	// domain name for this service directory
	string domain;

	// dataProductMap <domain, map<dataProductID, list<publishers> > >
    map<string, map<string, list<string> > > dataProductMap;

	// serviceMap <domain, map<serviceID, serviceProvider> >
    map<string, map<string, string> > serviceMap;

	// mapping from URL to name of publisher/service provider
	map<string, string> urlToComponentMap;

	// own GravityNode
    GravityNode gn;

    bool registeredPublishersReady, registeredPublishersProcessed;
    set<string> registerUpdatesToSend;

	void* context;
	SocketWithLock udpBroadcastSocket;
	SocketWithLock udpReceiverSocket;

	pthread_t udpBroadcasterThread;
	pthread_t udpReceiverThread;

	void sendBroadcasterParameters(string sdDomain, string url, unsigned int port, unsigned int rate);
	void sendReceiverParameters(string sdDomain, unsigned int port, unsigned int numValidDomains, string validDomains);

public:
    virtual ~ServiceDirectory();
    void start();
    shared_ptr<GravityDataProduct> request(const GravityDataProduct& dataProduct);
    shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);

private:
    void handleLookup(const GravityDataProduct& request, GravityDataProduct& response);
    void handleRegister(const GravityDataProduct& request, GravityDataProduct& response);
    void handleUnregister(const GravityDataProduct& request, GravityDataProduct& response);
    void addPublishers(const string &dataProductID, GravityDataProduct &response, const string &domain);
	void purgeObsoletePublishers(const string &dataProductID, const string &url);
};

} /* namespace gravity */
#endif /* SERVICEDIRECTORY_H_ */
