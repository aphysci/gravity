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
#include "protobuf/ComponentDataLookupResponsePB.pb.h"

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
	std::string domain;

	// dataProductMap <domain, map<dataProductID, list<publishers> > >
	std::map<std::string, std::map<std::string, std::list<gravity::PublisherInfoPB> > > dataProductMap;

	// serviceMap <domain, map<serviceID, serviceProvider> >
	std::map<std::string, std::map<std::string, std::string> > serviceMap;

	// mapping from URL to name of publisher/service provider
	std::map<std::string, std::string> urlToComponentMap;

	std::map<std::string, uint64_t> registrationInstanceMap;

	// mapping from Domain to URL
	std::map<std::string, std::string> domainMap;

	// own GravityNode
    GravityNode gn;

    bool registeredPublishersReady, registeredPublishersProcessed;
    std::set<std::string> registerUpdatesToSend;

    // needed to manage objects accessed from ServiceProvider thread
    Semaphore lock;

	void* context;
	SocketWithLock udpBroadcastSocket;
	SocketWithLock udpReceiverSocket;
	void* synchronizerSocket;

	void sendBroadcasterParameters(std::string sdDomain, std::string url, std::string ip, unsigned int port, unsigned int rate);
	void sendReceiverParameters(std::string sdDomain, std::string url, unsigned int port, unsigned int numValidDomains, std::string validDomains);
	void publishDomainUpdateMessage(std::string updateDomain, std::string url, ChangeType type);

	void updateProductLocations(std::string productID, std::string url, uint64_t timestamp, ChangeType changeType, RegistrationType registrationType);
	void updateProductLocations();
	std::tr1::shared_ptr<ServiceDirectoryMapPB> createOwnProviderMap();

public:
    virtual ~ServiceDirectory();
    void start();
    std::tr1::shared_ptr<GravityDataProduct> request(const GravityDataProduct& dataProduct);
    std::tr1::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);

private:
    void handleLookup(const GravityDataProduct& request, GravityDataProduct& response);
    void handleRegister(const GravityDataProduct& request, GravityDataProduct& response);
    void handleUnregister(const GravityDataProduct& request, GravityDataProduct& response);
    void addPublishers(const std::string &dataProductID, GravityDataProduct &response, const std::string &domain);
	void purgeObsoletePublishers(const std::string &dataProductID, const std::string &url);
};

} /* namespace gravity */
#endif /* SERVICEDIRECTORY_H_ */
