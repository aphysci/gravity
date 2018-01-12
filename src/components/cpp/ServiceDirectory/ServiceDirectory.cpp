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
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#include "ServiceDirectory.h"
#include "ServiceDirectoryUDPReceiver.h"
#include "ServiceDirectoryUDPBroadcaster.h"
#include "ServiceDirectorySynchronizer.h"
#include "GravityLogger.h"
#include "CommUtil.h"

#include "protobuf/ServiceDirectoryMapPB.pb.h"
#include "protobuf/ServiceDirectoryRegistrationPB.pb.h"
#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"
#include "protobuf/ServiceDirectoryResponsePB.pb.h"
#include "protobuf/ComponentLookupRequestPB.pb.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ComponentServiceLookupResponsePB.pb.h"
#include "protobuf/ServiceDirectoryDomainUpdatePB.pb.h"

//#include "protobuf/ServiceDirectoryBroadcastSetup.pb.h"

#include <zmq.h>
#include <boost/algorithm/string.hpp>

#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

#define REGISTERED_PUBLISHERS "RegisteredPublishers"
#define DIRECTORY_SERVICE "DirectoryService"

using namespace std;
using namespace std::tr1;

struct RegistrationData
{
    gravity::GravityNode* node;
    gravity::GravityServiceProvider* provider;
};

int main(void)
{
    gravity::ServiceDirectory serviceDirectory;
    serviceDirectory.start();
}

static void* registration(void* regData)
{
    // Extract GravityNode and GravityServiceProvider from arg structure
    gravity::GravityNode* gn = ((RegistrationData*)regData)->node;
    gravity::GravityServiceProvider* provider = ((RegistrationData*)regData)->provider;

    // Register data product for reporting changes to registered publishers
    gn->registerDataProduct(REGISTERED_PUBLISHERS, gravity::GravityTransportTypes::TCP);

    // Register the data product for domain broadcast details
	gn->registerDataProduct("ServiceDirectory_DomainDetails", gravity::GravityTransportTypes::TCP);
    
    // Register service for external requests
    gn->registerService(DIRECTORY_SERVICE, gravity::GravityTransportTypes::TCP, *provider);

	// Register the data products for adding and removing domains
	gn->registerDataProduct("ServiceDirectory_DomainUpdate",gravity::GravityTransportTypes::TCP);

    return NULL;
}

static void* startUDPBroadcastManager(void* context)
{

	// Create and start the UDP Broadcaster thread
	gravity::ServiceDirectoryUDPBroadcaster udpBroadcaster(context);
	udpBroadcaster.start();

	return NULL;
}

static void* startUDPReceiveManager(void* context)
{

	// Create and start the UDP receiver thread
	gravity::ServiceDirectoryUDPReceiver udpReceiver(context);
	udpReceiver.start();

	return NULL;
}

static void* startSynchronizer(void* args)
{
	// Create and start the thread to manage synchronization with other service directories
	gravity::SyncInitDetails* syncInitDetails = (gravity::SyncInitDetails*)args;
	gravity::ServiceDirectorySynchronizer synchronizer(syncInitDetails->context, syncInitDetails->url);
	synchronizer.start();

	return NULL;
}

static bool validateDomainName(string domain)
{
	// Validate domain name
	bool valid = true;
	std::locale loc;
	for (std::string::iterator it = domain.begin(); it != domain.end(); ++it)
	{
		if (!std::isalnum(*it, loc) && *it != '.')
		{
			valid = false;
			break;
		}
	}

	return valid;
}

static int parseDomainCSV(string &csv)
{
	int prevPos=0;
	unsigned int pos = csv.find(",",0);
	int commaCount = 0;

	if(csv.length()==0)
	{
		return 0;
	}

	while(pos != string::npos)
	{
		if(!validateDomainName(csv.substr(prevPos,pos-prevPos)))
		{
			return -1;
		}
		commaCount++;
		prevPos=pos+1;
		pos=csv.find(",",pos+1);
	}

	return commaCount+1;	
}

namespace gravity
{

ServiceDirectory::~ServiceDirectory()
{
}

void ServiceDirectory::start()
{
	//set up zmq context
	context = zmq_init(1);

	//set up broadcast and recieve managers
	if (!context)
	{
		Log::fatal("Could not create ZMQ Context");
		return;
	}
	
    registeredPublishersReady = registeredPublishersProcessed = false;
    gn.init("ServiceDirectory");

    std::string sdURL = gn.getStringParam("ServiceDirectoryUrl", "tcp://*:5555");
    boost::replace_all(sdURL, "localhost", "127.0.0.1");
    Log::message("running with SD connection string: %s", sdURL.c_str());

	bool broadcastEnabled = gn.getBoolParam("BroadcastEnabled",false);

	// Get the optional domain for this Service Directory instance
	domain = gn.getStringParam("Domain", "");
	
	if (!validateDomainName(domain))
	{
		domain = "";
		Log::warning("Invalid Domain (must be alpha-numeric or '.').");

		//don't broadcast if domain is invalid
		broadcastEnabled = false;
	}
	Log::message("Domain set to '%s'", domain.c_str());


	unsigned int broadcastPort = gn.getIntParam("ServiceDirectoryBroadcastPort",DEFAULT_BROADCAST_PORT);



	string knownDomainCSV = gn.getStringParam("DomainSyncList","");
	int  numDomains = parseDomainCSV(knownDomainCSV);

	if(numDomains < 0)
	{
		knownDomainCSV="";
		numDomains=0;
		Log::warning("Invalid DomainSyncList (must be alpha-numeric or '.').");
	}
	Log::message("DomainSyncList set to '%s'",knownDomainCSV.c_str());

    void *context = zmq_init(1);
    if (!context)
    {
        Log::critical("Could not create ZeroMQ context, exiting");
        exit(1);
    }

    // Set up the inproc socket to listen for requests messages from the GravityNode
    void *socket = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(socket, sdURL.c_str());
    if (rc < 0)
    {
        Log::fatal("Could not bind address for ServiceDirectory, error code was %d (%s)", rc, strerror(errno));
        exit(1);
    }

	std::vector<zmq_pollitem_t> pollItems;

    // Always have at least the gravity node to poll
    zmq_pollitem_t pollItem;
    pollItem.socket = socket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;
	pollItems.push_back(pollItem);

	//If broadcast was enabled, start the broadcaster
	if(broadcastEnabled)
	{
		Log::message("Starting ServiceDirectoryBroadcaster");

		string broadcastIP = gn.getStringParam("ServiceDirectoryBroadcastIP", "");
		unsigned int broadcastRate = gn.getIntParam("ServiceDirectoryBroadcastRate",DEFAULT_BROADCAST_RATE_SEC);

		//start the udp broadcaster
		udpBroadcastSocket.socket = zmq_socket(context,ZMQ_REQ);
		zmq_bind(udpBroadcastSocket.socket,"inproc://service_directory_udp_broadcast");
		pthread_create(&udpBroadcasterThread,NULL,startUDPBroadcastManager,context);

		//configure the broadcaster
		sendBroadcasterParameters(domain,sdURL,broadcastIP,broadcastPort,broadcastRate);
	}

	SyncInitDetails syncInitDetails;

	//only start the receiver is there is at least one domain to sync to
	if(numDomains > 0)
	{
		Log::message("Starting ServiceDirectoryReceiver");
		//create the socket for receiving domains from the udp receiver, used for polling
		void *domainRecvSocket = zmq_socket(context,ZMQ_SUB);
		zmq_bind(domainRecvSocket,"inproc://service_directory_domain_socket");
		zmq_setsockopt(domainRecvSocket,ZMQ_SUBSCRIBE,NULL,0);

		//start the udp receiver
		udpReceiverSocket.socket = zmq_socket(context,ZMQ_REQ);
		zmq_bind(udpReceiverSocket.socket,"inproc://service_directory_udp_receive");
		pthread_create(&udpReceiverThread,NULL,startUDPReceiveManager,context);

		//configure the receiver
		sendReceiverParameters(domain,sdURL,broadcastPort,numDomains,knownDomainCSV);

		// start the synchronization thread
		Log::message("Starting ServiceDirectorySynchronization thread");
		syncInitDetails.context = context;
		syncInitDetails.url = sdURL;
		pthread_create(&synchronizerThread, NULL, startSynchronizer, (void*)&syncInitDetails);
		
		// Configure the comms channel to direct the synchronizer thread
		synchronizerSocket = zmq_socket(context, ZMQ_PUB);
		zmq_bind(synchronizerSocket, "inproc://service_directory_synchronizer");			

		// Poll the UDP Receiver
		zmq_pollitem_t udpRecvPollItem;
		udpRecvPollItem.socket=domainRecvSocket;
		udpRecvPollItem.events=ZMQ_POLLIN;
		udpRecvPollItem.fd=0;
		udpRecvPollItem.revents=0;
		pollItems.push_back(udpRecvPollItem);
	}

    /*************
     * IMPORTANT: The following block of code should be the last thing the SD does before entering the main loop.
     *
     * This registration thread will register data products and services using the SD's GravityNode.  After this
     * registration thread is started, it is assumed that the only actions on the GravityNode in this thread will be calls
     * to publish.  These calls are all thread safe.
     *
     * We cannot synchronize access to the GravityNode here because the registration calls require the SD to service
     * the registration requests in the loop below.
     *
     * Calls FROM the GravityNode (i.e. ServiceProvider::request below) won't cause deadlock as long as we don't make calls to GravityNode,
     * but because it is a separate thread, we need to manage data access.
     */
    struct RegistrationData regData;
    regData.node = &gn;
    regData.provider = this;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) == 0)
    {
        pthread_t registerThread;
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&registerThread, &attr, registration, (void*)&regData);
        pthread_attr_destroy(&attr);
    }
    /****
     * End last block of code before main loop
     */

    // Process forever...
    while (true)
    {
        // Start polling socket(s), blocking while we wait
        int rc = zmq_poll(&pollItems[0], pollItems.size(), -1); // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            break;
        }

        // Process new subscription requests from the gravity node
        if (pollItems[0].revents & ZMQ_POLLIN)
        {
            // Read the data product
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_recvmsg(pollItem.socket, &msg, -1);
            GravityDataProduct req(zmq_msg_data(&msg), zmq_msg_size(&msg));
            zmq_msg_close(&msg);

            /**
             * Most of the methods in this class are called on the main thread, and are invoked from within
             * this locked block.  If that changes, additional locked blocks may be needed.  Currently the only
             * other locked block needed is in the ServiceProvider::request method defined below which runs
             * on a separate thread provided by the GravityNode.
             */
            lock.Lock();
            shared_ptr<GravityDataProduct> response = request(req);
            if (!registeredPublishersProcessed && registeredPublishersReady)
            {
                GravityDataProduct update(REGISTERED_PUBLISHERS);
                for (set<string>::iterator it = registerUpdatesToSend.begin(); it != registerUpdatesToSend.end(); it++)
                {
                    Log::debug("Processing pending registered publishers update for %s", it->c_str());
                    addPublishers(*it, update, domain);
                    gn.publish(update, *it);
                }
                registerUpdatesToSend.clear();
                registeredPublishersProcessed = true;
            }
            lock.Unlock();

            sendGravityDataProduct(pollItem.socket, *response, ZMQ_DONTWAIT);
        }

		if(numDomains > 0)
		{
			//process domain commands
			if(pollItems[1].revents & ZMQ_POLLIN)
			{
				void* domainSocket = pollItems[1].socket;

				string command = readStringMessage(domainSocket);

				if (command == "Add")
				{
					string domainToAdd = readStringMessage(domainSocket);
					string url = readStringMessage(domainSocket);

					domainMap[domainToAdd] = url;
		
					// publish domain added message
					publishDomainUpdateMessage(domainToAdd,url,ADD);

					// Send Add command to synchronization thread
					Log::debug("Sending add command to synchronziation thread for %s:%s", domainToAdd.c_str(), url.c_str());
					sendStringMessage(synchronizerSocket, "Add", ZMQ_SNDMORE);
					sendStringMessage(synchronizerSocket, domainToAdd, ZMQ_SNDMORE);
					sendStringMessage(synchronizerSocket, url, ZMQ_DONTWAIT);

				}
				else if (command == "Remove")
				{
					string domainToRemove = readStringMessage(domainSocket);
					string url = domainMap[domainToRemove];

					domainMap.erase(domainToRemove);

					// publish domain removed message
					publishDomainUpdateMessage(domainToRemove,url,REMOVE);

					// Send Remove command to synchronization thread
					Log::debug("Sending remove command to synchronziation thread for %s", domainToRemove.c_str());
					sendStringMessage(synchronizerSocket, "Remove", ZMQ_SNDMORE);
					sendStringMessage(synchronizerSocket, domainToRemove, ZMQ_DONTWAIT);
				}
			}
		}
    }
}

shared_ptr<GravityDataProduct> ServiceDirectory::request(const GravityDataProduct& dataProduct)
{
	shared_ptr<GravityDataProduct> gdpResponse = shared_ptr<GravityDataProduct>(new GravityDataProduct("DataProductRegistrationResponse"));
    string requestType = dataProduct.getDataProductID();

    ///////////////////////////////////
    // All requests are coming in via a single thread, so not performing any explicit
    // synchronization here
    if (requestType == "ComponentLookupRequest")
    {
        Log::trace("Handling lookup");
        handleLookup(dataProduct, *gdpResponse);
    }
    else if (requestType == "RegistrationRequest")
    {
        Log::trace("Handling register");
        handleRegister(dataProduct, *gdpResponse);
    }
    else if (requestType == "UnregistrationRequest")
    {
        Log::trace("Handling unregister");
        handleUnregister(dataProduct, *gdpResponse);
    }
    else if (requestType == "GetDomain")
    {
        gdpResponse->setData(domain.c_str(), domain.length());
    }
    else
    {
        Log::warning("unknown request type: %s", requestType.c_str());
    }

    return gdpResponse;
}

/**
 * Because the lock in this method blocks the main SD loop above, we can't issue any GravityNode calls
 * here that require SD support (e.g. register, etc).
 */
shared_ptr<GravityDataProduct> ServiceDirectory::request(const std::string serviceID, const GravityDataProduct& request)
{
    Log::debug("Received service request of type '%s'", serviceID.c_str());
	shared_ptr<GravityDataProduct> gdpResponse = shared_ptr<GravityDataProduct>(new GravityDataProduct("DirectoryServiceResponse"));
   
    if (serviceID == DIRECTORY_SERVICE)
    {
        lock.Lock();
        string requestType = request.getDataProductID();
        if (requestType == "GetProviders")
        {
            // Build map
            ServiceDirectoryMapPB providerMap;
            providerMap.set_domain(domain);

			for (map<string, map<string, string> >::iterator it = serviceMap.begin(); it != serviceMap.end(); it++)
			{
				string domain = it->first;
				map<string,string> sMap = it->second;
				for (map<string,string>::iterator it2 = sMap.begin(); it2 != sMap.end(); it2++)
				{
					ProductLocations* locs = providerMap.add_service_provider();
					locs->set_product_id(it2->first); // service ID
					locs->add_url(it2->second); // url
					locs->add_component_id(urlToComponentMap[it2->second]); // component id
					locs->add_timestamp(registrationInstanceMap[it2->second]); //timestamp
					locs->add_domain_id(domain); // domain
				}
			}            
            
			for (map<string, map<string, list<PublisherInfoPB> > >::iterator it = dataProductMap.begin(); it != dataProductMap.end(); it++)
			{
				string domain = it->first;
				map<string, list<PublisherInfoPB> > dpMap = it->second;
				for(map<string, list<PublisherInfoPB> >::iterator it = dpMap.begin(); it != dpMap.end(); it++)
				{
					ProductLocations* locs = providerMap.add_data_provider();
					locs->set_product_id(it->first); // data product ID
					locs->add_domain_id(domain); // domain
					list<PublisherInfoPB> urls = it->second;
					for(list<PublisherInfoPB>::iterator lit = urls.begin(); lit != urls.end(); lit++)
					{
						locs->add_url(lit->url()); // url
						locs->add_component_id(urlToComponentMap[lit->url()]); // component id
						locs->add_timestamp(registrationInstanceMap[lit->url()]); //timestamp
					}
				}
			}            

            // Place map on response data product
            gdpResponse->setData(providerMap);
        }
		else if (requestType == "GetDomain")
        {
			gdpResponse->setData(domain.c_str(), domain.length());
		}
        lock.Unlock();
    }

    return gdpResponse;
}

void ServiceDirectory::handleLookup(const GravityDataProduct& request, GravityDataProduct& response)
{
    ComponentLookupRequestPB lookupRequest;
    request.populateMessage(lookupRequest);

	// Get the requested domain (defaulting to our own)
	string lookupDomain = domain;
	if (lookupRequest.has_domain_id() && !lookupRequest.domain_id().empty())
	{
		lookupDomain = lookupRequest.domain_id();
	}

    //NOTE: 0MQ does not have a concept of who the message was sent from so that info is lost.    
    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
    {
		// Get data product map for requested domain (defaulting to our own)
		if (dataProductMap.count(lookupDomain) > 0)
		{
			map<string, list<PublisherInfoPB> > dpMap = dataProductMap[lookupDomain];

			Log::message("[Lookup Request] ID: %s, Domain: %s, MessageType: Data Product, First Server: %s", 
					 lookupRequest.lookupid().c_str(),
					 lookupDomain.c_str(),
                     dpMap.count(lookupRequest.lookupid()) != 0 ?
                     dpMap[lookupRequest.lookupid()].front().url().c_str(): "");
		}
		else
		{
			Log::message("[Lookup Request] No data provider found for ID: %s in Domain: %s",
						lookupRequest.lookupid().c_str(),
						lookupDomain.c_str());
		}

		addPublishers(lookupRequest.lookupid(), response, lookupDomain);
    }
    else
    {
		ComponentServiceLookupResponsePB lookupResponse;
		lookupResponse.set_lookupid(lookupRequest.lookupid());        
		lookupResponse.set_url("");
		lookupResponse.set_domain_id(lookupDomain);

		if (serviceMap.count(lookupDomain) > 0)
		{
			// Get service map for our domain
			map<string, string> sMap = serviceMap[lookupDomain];

			Log::message("[Lookup Request] ID: %s, MessageType: Service, Server: %s", lookupRequest.lookupid().c_str(),
                     sMap.count(lookupRequest.lookupid()) != 0 ?
                     sMap[lookupRequest.lookupid()].c_str(): "");

			if (sMap.count(lookupRequest.lookupid()) > 0)
			{
				lookupResponse.set_url(sMap[lookupRequest.lookupid()]);
			}
		}

        response.setData(lookupResponse);
    }
}

void ServiceDirectory::updateProductLocations(string productID, string url, uint64_t timestamp, ChangeType changeType, RegistrationType registrationType)
{
	ServiceDirectoryMapPB providerMap;
	providerMap.set_domain(domain);

	map<string,string> sMap = serviceMap[domain];	
	for (map<string,string>::iterator it2 = sMap.begin(); it2 != sMap.end(); it2++)
	{
		ProductLocations* locs = providerMap.add_service_provider();
		locs->set_product_id(it2->first); // service ID
		locs->add_url(it2->second); // url
		locs->add_component_id(urlToComponentMap[it2->second]); // component id
		locs->add_timestamp(registrationInstanceMap[it2->second]); //timestamp
		locs->add_domain_id(domain); // domain
	}            
            
	map<string, list<PublisherInfoPB> > dpMap = dataProductMap[domain];
	for(map<string, list<PublisherInfoPB> >::iterator it = dpMap.begin(); it != dpMap.end(); it++)
	{
		ProductLocations* locs = providerMap.add_data_provider();
		locs->set_product_id(it->first); // data product ID
		locs->add_domain_id(domain); // domain
		list<PublisherInfoPB> urls = it->second;
		for(list<PublisherInfoPB>::iterator lit = urls.begin(); lit != urls.end(); lit++)
		{
			locs->add_url(lit->url()); // url
			locs->add_component_id(urlToComponentMap[lit->url()]); // component id
			locs->add_timestamp(registrationInstanceMap[lit->url()]); // timestamp
		}
	}
	
	// Add change to data product
	Log::debug("Adding Change : %s %s %s %lu", productID.c_str(), url.c_str(), 
			urlToComponentMap[url].c_str(),0);//urlToComponentMap[url].timestamp); 
	ProductChange* change = providerMap.mutable_change();
	change->set_product_id(productID);
	change->set_url(url);
	change->set_component_id(urlToComponentMap[url]);
	change->set_timestamp(timestamp);
    change->set_change_type(changeType == REMOVE ? ProductChange_ChangeType_REMOVE : ProductChange_ChangeType_ADD);
    change->set_registration_type(registrationType == SERVICE ? 
                    ProductChange_RegistrationType_SERVICE : ProductChange_RegistrationType_DATA);

	// Publish update
	Log::debug("Publishing ServiceDirectory_DomainDetails");
	GravityDataProduct gdp("ServiceDirectory_DomainDetails");
	gdp.setData(providerMap);
	gn.publish(gdp);
}

void ServiceDirectory::handleRegister(const GravityDataProduct& request, GravityDataProduct& response)
{
    ServiceDirectoryRegistrationPB registration;
    request.populateMessage(registration);
    bool foundDup = false;

    // If the registration does not specify a domain, default to our own
	string domain = registration.has_domain() ? registration.domain() : this->domain;
    
	bool update = true;

	// if the request does not have a timestamp
	if(!registration.has_timestamp())
	{
		Log::warning("Received Register for URL: %s with no timestamp. Ignoring Request",registration.url().c_str());
		update = false;
	}
	//if we already have an instance for this URL
	else if(registrationInstanceMap.find(registration.url())!=registrationInstanceMap.end())
	{
		//check if the update for the specified URL is older then our current mapping
		if(registration.timestamp() < registrationInstanceMap[registration.url()])
		{
			Log::warning("Received Register for URL: %s with old timestamp %lu",registration.url().c_str(),registration.timestamp());
			update = false;
		}
	}

	if(update)
	{
		if (registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA)
		{
			map<string, list<PublisherInfoPB> >& dpMap = dataProductMap[domain];

			if (registration.id() == REGISTERED_PUBLISHERS)
			{
				// When this request is received here, this product ID has already been registered with our GravityNode, so
				// it's safe to start publishing to it (cached values will be sent to new subscribers).
				registeredPublishersReady = true;
			}
			list<PublisherInfoPB>& urls = dpMap[registration.id()];
			list<PublisherInfoPB>::iterator iter = urls.begin();
			for (;iter != urls.end();iter++)
			{
				if (iter->url() == registration.url())
					break;
			}
			if (iter == urls.end())
			{
				PublisherInfoPB infoPB;
				infoPB.set_url(registration.url());
				dpMap[registration.id()].push_back(infoPB);
				if (registration.is_relay())
				{
					Log::debug("Registering relay for %s", registration.id().c_str());
				}
				
				urlToComponentMap[registration.url()] = registration.component_id();

				if (registeredPublishersReady)
				{
					GravityDataProduct update(REGISTERED_PUBLISHERS);
					addPublishers(registration.id(), update, domain);
					gn.publish(update, registration.id());
				}
				else
				{
					registerUpdatesToSend.insert(registration.id());
				}

				// Remove any previous registrations at this URL as they obviously no longer exist
				purgeObsoletePublishers(registration.id(), registration.url());

				//insert new instance mapping for URL
				registrationInstanceMap[registration.url()] = registration.timestamp();

				// Update any subscribers interested in our providers
				if (domain == this->domain && registration.id() != REGISTERED_PUBLISHERS)
				{
					Log::debug("Sending update of product definitions (resulting from added data for '%s' @ '%s')", registration.id().c_str(), registration.url().c_str());
					updateProductLocations(registration.id(), registration.url(), registration.timestamp(), ADD, DATA);
				}
			}
			else
			{
				foundDup = true;
						
				//Replace existing timestamp with the new one
				registrationInstanceMap[registration.url()] = registration.timestamp();
				
				// Update any subscribers interested in our providers
				if(domain == this->domain)
				{
					updateProductLocations(registration.id(), registration.url(), registration.timestamp(), ADD, DATA);
				}
			}
		}
		else
		{
			map<string, string>& sMap = serviceMap[domain];

			if (sMap.find(registration.id()) != sMap.end())
			{
				Log::warning("Replacing existing provider for service id '%s'", registration.id().c_str()); 
			}
			// Add as service provider, overwriting any existing provider for this service
			sMap[registration.id()] = registration.url();
			
			urlToComponentMap[registration.url()] = registration.component_id();
			
			// Remove any previous publisher registrations at this URL as they obviously no longer exist
			purgeObsoletePublishers(registration.id(), registration.url());

			//insert new instance mapping for URL
			registrationInstanceMap[registration.url()] = registration.timestamp();


			// Update any subscribers interested in our providers
			if (domain == this->domain)
			{
				Log::debug("Sending update of product definitions (resulting from added service for '%s' @ '%s')", registration.id().c_str(), registration.url().c_str());
				updateProductLocations(registration.id(), registration.url(), registration.timestamp(), ADD, SERVICE);
			}
		}
		Log::message("[Register] ID: %s, MessageType: %s, URL: %s, Domain: %s", registration.id().c_str(),
				registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA ? "Data Product": "Service", 
				registration.url().c_str(), registration.domain().c_str());
	
	}

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(registration.id());
    if (foundDup)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::SUCCESS);
        Log::warning("Attempt to register duplicate url (%s) for %s", registration.url().c_str(), registration.id().c_str());
    }
    else
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::SUCCESS);
    }

    response.setData(sdr);
}

void ServiceDirectory::handleUnregister(const GravityDataProduct& request, GravityDataProduct& response)
{
    ServiceDirectoryUnregistrationPB unregistration;
    request.populateMessage(unregistration);
    bool foundUrl = true;

    // If the registration does not specify a domain, default to our own
	string domain = unregistration.has_domain() ? unregistration.domain() : this->domain;

    if (unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA)
    {
		map<string, list<PublisherInfoPB> >& dpMap = dataProductMap[domain];

        list<PublisherInfoPB>* infoPBs = &dpMap[unregistration.id()];
        list<PublisherInfoPB>::iterator iter = infoPBs->begin();
        for (;iter != infoPBs->end(); iter++)
        {
        	if (iter->url() == unregistration.url())
        		break;
        }
        if (iter != infoPBs->end())
        {
            dpMap[unregistration.id()].erase(iter);
			if (dpMap[unregistration.id()].empty())
			{
				dpMap.erase(unregistration.id());
			}

			if (registeredPublishersReady)
			{
			    GravityDataProduct update(REGISTERED_PUBLISHERS);
			    addPublishers(unregistration.id(), update, domain);
			    gn.publish(update, unregistration.id());
			}
			else
			{
			    registerUpdatesToSend.insert(unregistration.id());
			}

            // Update any subscribers interested in our providers
            if (domain == this->domain)
            {
				updateProductLocations(unregistration.id(), unregistration.url(), 
						registrationInstanceMap[unregistration.url()], REMOVE, DATA);
            }
					
			//remove instance mapping
			registrationInstanceMap.erase(unregistration.url());
        }
        else
        {
            foundUrl = false;
        }
    }
    else
    {
		map<string, string>& sMap = serviceMap[domain];

        if (sMap.find(unregistration.id()) != sMap.end())
        {
            sMap.erase(unregistration.id());

            // Update any subscribers interested in our providers
            if (domain == this->domain)
            {
                updateProductLocations(unregistration.id(), unregistration.url(), 
						registrationInstanceMap[unregistration.url()], REMOVE, SERVICE);
            }

			// remove instance mapping
			registrationInstanceMap.erase(unregistration.url());
        }
        else
        {
            foundUrl = false;
        }
    }

    Log::message("[Unregister] ID: %s, MessageType: %s, URL: %s, Domain: %s", unregistration.id().c_str(),
            unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA? "Data Product": "Service", 
            unregistration.url().c_str(), unregistration.domain().c_str());

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(unregistration.id());
    if (!foundUrl)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::NOT_REGISTERED);
        Log::warning("Attempt to unregister unregistered %s, %s, %s", unregistration.id().c_str(), 
            unregistration.url().c_str(), unregistration.domain().c_str());
    }
    else
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::SUCCESS);
    }

    response.setData(sdr);
}

void ServiceDirectory::purgeObsoletePublishers(const string &dataProductID, const string &url)
{
	map<string, list<PublisherInfoPB> >& dpMap = dataProductMap[domain];
	map<string,list<PublisherInfoPB> >::iterator iter = dpMap.begin();
	while (iter != dpMap.end())
    {		
        if (iter->first != dataProductID)
        {
            list<PublisherInfoPB>& infoPBs = iter->second;
            list<PublisherInfoPB>::iterator it = infoPBs.begin();
            for (;it != infoPBs.end(); it++)
            {
            	if (it->url() == url)
            		break;
            }
            if (it != infoPBs.end())
            {
                // We need to remove/"unregister" this one
                Log::message("[Auto-Unregister] ID: %s, MessageType: Data Product, URL: %s",
                                iter->first.c_str(), url.c_str());
                infoPBs.erase(it);

                if (registeredPublishersReady)
                {
                    GravityDataProduct update(REGISTERED_PUBLISHERS);
                    addPublishers(iter->first, update, domain);
                    gn.publish(update, iter->first);
                }
                else
                {
                    registerUpdatesToSend.insert(iter->first);
                }

                // Update any subscribers interested in our providers
                updateProductLocations(iter->first, url, registrationInstanceMap[url], REMOVE, DATA);

				//remove old Instance Mapping
				registrationInstanceMap.erase(url);
            }
        }

		if (iter->second.empty())
		{
			dpMap.erase(iter++);
		}
		else
		{
			++iter;
		}
	}	
}

void ServiceDirectory::addPublishers(const string &dataProductID, GravityDataProduct &response, const string &domain)
{
	map<string, list<PublisherInfoPB> > dpMap = dataProductMap[domain];
    ComponentDataLookupResponsePB lookupResponse;
    lookupResponse.set_lookupid(dataProductID);
	lookupResponse.set_domain_id(domain);
	if (dpMap.count(dataProductID) != 0)
	{
		list<PublisherInfoPB>* infoPBs = &dpMap[dataProductID];
		for (list<PublisherInfoPB>::iterator iter = infoPBs->begin(); iter != infoPBs->end(); iter++)
		{
			PublisherInfoPB* lookupInfoPB = lookupResponse.add_publishers();
			lookupInfoPB->CopyFrom(*iter);
		}
	}
    response.setData(lookupResponse);
}

void ServiceDirectory::sendBroadcasterParameters(string sdDomain, string url, string ip, unsigned int port, unsigned int rate)
{
	udpBroadcastSocket.lock.Lock();

	sendStringMessage(udpBroadcastSocket.socket,"broadcast",ZMQ_SNDMORE);
	sendStringMessage(udpBroadcastSocket.socket,sdDomain,ZMQ_SNDMORE);
	sendStringMessage(udpBroadcastSocket.socket,url,ZMQ_SNDMORE);
	sendStringMessage(udpBroadcastSocket.socket,ip,ZMQ_SNDMORE);
	
	zmq_msg_t msg;
	zmq_msg_init_size(&msg,sizeof(port));
	memcpy(zmq_msg_data(&msg),&port,sizeof(port));
	zmq_sendmsg(udpBroadcastSocket.socket,&msg,ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	zmq_msg_t msg2;
	zmq_msg_init_size(&msg2,sizeof(rate));
	memcpy(zmq_msg_data(&msg2),&rate,sizeof(rate));
	zmq_sendmsg(udpBroadcastSocket.socket,&msg2,ZMQ_DONTWAIT);
	zmq_msg_close(&msg2);

	udpBroadcastSocket.lock.Unlock();
}

void ServiceDirectory::sendReceiverParameters(string sdDomain, string url, unsigned int port, unsigned int numValidDomains, string validDomains)
{
	udpReceiverSocket.lock.Lock();

	sendStringMessage(udpReceiverSocket.socket,"receive",ZMQ_SNDMORE);
	sendStringMessage(udpReceiverSocket.socket,sdDomain,ZMQ_SNDMORE);
	sendStringMessage(udpReceiverSocket.socket,url,ZMQ_SNDMORE);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg,sizeof(port));
	memcpy(zmq_msg_data(&msg),&port,sizeof(port));
	zmq_sendmsg(udpReceiverSocket.socket,&msg,ZMQ_SNDMORE);
	zmq_msg_close(&msg);

	zmq_msg_t msg2;
	zmq_msg_init_size(&msg2,sizeof(numValidDomains));
	memcpy(zmq_msg_data(&msg2),&numValidDomains,sizeof(numValidDomains));
	zmq_sendmsg(udpReceiverSocket.socket,&msg2,ZMQ_SNDMORE);
	zmq_msg_close(&msg2);

	sendStringMessage(udpReceiverSocket.socket,validDomains,ZMQ_DONTWAIT);

	udpReceiverSocket.lock.Unlock();
}

void ServiceDirectory::publishDomainUpdateMessage(string updateDomain, string url, ChangeType type)
{
	ServiceDirectoryDomainUpdatePB updatePB;

	//add all the currently synced domain to the message
	map<string,string>::iterator iter = domainMap.begin();
	while (iter != domainMap.end())
    {	
		updatePB.add_known_domains(iter->first);
		++iter;
	}

	updatePB.set_update_domain(updateDomain);

	updatePB.set_type(type == ADD? ServiceDirectoryDomainUpdatePB_UpdateType_ADD : 
		ServiceDirectoryDomainUpdatePB_UpdateType_REMOVE);

	GravityDataProduct gdp("ServiceDirectory_DomainUpdate");
	gdp.setData(updatePB);
	gn.publish(gdp);

}

} /* namespace gravity */
