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
#include "protobuf/ServiceDirectoryMapPB.pb.h"

using namespace std;

namespace gravity
{

enum RegistrationType
{
	SERVICE,
	DATA
};

enum ChangeType
{
	REMOVE,
	ADD
};

class ServiceDirectory : GravityServiceProvider
{
private:

	// domain name for this service directory
	string domain;

	// dataProductMap <domain, map<dataProductID, list<publishers> > >
    map<string, map<string, list<string> > > dataProductMap;

	// serviceMap <domain, map<serviceID, serviceProvider> >
    map<string, map<string, string> > serviceMap;

	// mapping from URL to name of publisher/service provider
	map<string, string> urlToComponentMap;

	map<string, uint64_t> registrationInstanceMap;

	// mapping from Domain to URL
	map<string, string> domainMap;

	// own GravityNode
    GravityNode gn;

    bool registeredPublishersReady, registeredPublishersProcessed;
    set<string> registerUpdatesToSend;

	void* context;
	SocketWithLock udpBroadcastSocket;
	SocketWithLock udpReceiverSocket;
	void* synchronizerSocket;

	pthread_t udpBroadcasterThread;
	pthread_t udpReceiverThread;
	pthread_t synchronizerThread;

	void sendBroadcasterParameters(string sdDomain, string url, unsigned int port, unsigned int rate);
	void sendReceiverParameters(string sdDomain, string url, unsigned int port, unsigned int numValidDomains, string validDomains);
	void publishDomainUpdateMessage(string updateDomain, string url, ChangeType type);

	void updateProductLocations(string productID, string url, uint64_t timestamp, ChangeType changeType, RegistrationType registrationType);
	void updateProductLocations();
	shared_ptr<ServiceDirectoryMapPB> createOwnProviderMap();

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
