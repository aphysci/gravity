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
#include "GravityLogger.h"
#include "CommUtil.h"

#include "protobuf/ServiceDirectoryMapPB.pb.h"
#include "protobuf/ServiceDirectoryRegistrationPB.pb.h"
#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"
#include "protobuf/ServiceDirectoryResponsePB.pb.h"
#include "protobuf/ComponentLookupRequestPB.pb.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ComponentServiceLookupResponsePB.pb.h"
#include <zmq.h>
#include <boost/algorithm/string.hpp>

#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

#define REGISTERED_PUBLISHERS "RegisteredPublishers"
#define DIRECTORY_SERVICE "DirectoryService"

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
    
    // Register service for external requests
    gn->registerService(DIRECTORY_SERVICE, gravity::GravityTransportTypes::TCP, *provider);

    return NULL;
}

namespace gravity
{

ServiceDirectory::~ServiceDirectory()
{
}

void ServiceDirectory::start()
{
    gn.init("ServiceDirectory");

    std::string sdURL = gn.getStringParam("ServiceDirectoryUrl", "tcp://*:5555");
    boost::replace_all(sdURL, "localhost", "127.0.0.1");

    Log::message("running with SD connection string: %s", sdURL.c_str());

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
        Log::fatal("Could bind address for ServiceDirectory, error code was %d (%s)", rc, strerror(errno));
        exit(1);
    }

    // Always have at least the gravity node to poll
    zmq_pollitem_t pollItem;
    pollItem.socket = socket;
    pollItem.events = ZMQ_POLLIN;
    pollItem.fd = 0;
    pollItem.revents = 0;

    // register the data product & service in another thread so we can process request below
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

    // Process forever...
    while (true)
    {
        // Start polling socket(s), blocking while we wait
        int rc = zmq_poll(&pollItem, 1, -1); // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            break;
        }

        // Process new subscription requests from the gravity node
        if (pollItem.revents & ZMQ_POLLIN)
        {
            // Read the data product
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_recvmsg(pollItem.socket, &msg, -1);
            GravityDataProduct req(zmq_msg_data(&msg), zmq_msg_size(&msg));
            zmq_msg_close(&msg);

            shared_ptr<GravityDataProduct> response = request(req);

            sendGravityDataProduct(pollItem.socket, *response, ZMQ_DONTWAIT);
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
    else
    {
        Log::warning("unknown request type: %s", requestType.c_str());
    }

    return gdpResponse;
}

shared_ptr<GravityDataProduct> ServiceDirectory::request(const std::string serviceID, const GravityDataProduct& request)
{
    Log::debug("Received service request of type '%s'", serviceID.c_str());
	shared_ptr<GravityDataProduct> gdpResponse = shared_ptr<GravityDataProduct>(new GravityDataProduct("DirectoryServiceResponse"));
   
    if (serviceID == "DirectoryService") 
    {
        string requestType = request.getDataProductID();
        if (requestType == "GetProviders")
        {
            // Build map
            ServiceDirectoryMapPB providerMap;
            for(map<string,string>::iterator it = serviceMap.begin(); it != serviceMap.end(); it++) 
            {
                ProductLocations* locs = providerMap.add_service_provider();
                locs->set_product_id(it->first); // service ID
                locs->add_url(it->second); // url
            }
            
            for(map<string, list<string> >::iterator it = dataProductMap.begin(); it != dataProductMap.end(); it++) 
            {
                ProductLocations* locs = providerMap.add_data_provider();
                locs->set_product_id(it->first); // data product ID
                list<string> urls = it->second;
                for(list<string>::iterator lit = urls.begin(); lit != urls.end(); lit++) 
                {
                    locs->add_url(*lit); // url
                }
            }

            // Place map on response data product
            gdpResponse->setData(providerMap);
        }
    }

    return gdpResponse;
}

void ServiceDirectory::handleLookup(const GravityDataProduct& request, GravityDataProduct& response)
{
    ComponentLookupRequestPB lookupRequest;
    request.populateMessage(lookupRequest);
    //NOTE: 0MQ does not have a concept of who the message was sent from so that info is lost.
    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
    {
        Log::message("[Lookup Request] ID: %s, MessageType: Data Product, First Server: %s", lookupRequest.lookupid().c_str(),
                     dataProductMap.count(lookupRequest.lookupid()) != 0 ?
                     dataProductMap[lookupRequest.lookupid()].front().c_str(): "");
    }
    else
    {
        Log::message("[Lookup Request] ID: %s, MessageType: Service, Server: %s", lookupRequest.lookupid().c_str(),
                     serviceMap.count(lookupRequest.lookupid()) != 0 ?
                     serviceMap[lookupRequest.lookupid()].c_str(): "");
    }
    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
    {
        addPublishers(lookupRequest.lookupid(), response);
    }
    else
    {
        ComponentServiceLookupResponsePB lookupResponse;
        lookupResponse.set_lookupid(lookupRequest.lookupid());
        lookupResponse.set_url(serviceMap[lookupRequest.lookupid()]);
        response.setData(lookupResponse);
    }
}

void ServiceDirectory::handleRegister(const GravityDataProduct& request, GravityDataProduct& response)
{
    ServiceDirectoryRegistrationPB registration;
    request.populateMessage(registration);
    bool foundDup = false;
    if (registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA)
    {
        list<string>* urls = &dataProductMap[registration.id()];
        list<string>::iterator iter = find(urls->begin(), urls->end(), registration.url());
        if (iter == urls->end())
        {
            dataProductMap[registration.id()].push_back(registration.url());
            GravityDataProduct update(REGISTERED_PUBLISHERS);
            addPublishers(registration.id(), update);
            gn.publish(update, registration.id());

            // Remove any previous registrations at this URL as they obviously no longer exist
            purgeObsoletePublishers(registration.id(), registration.url());
        }
        else
        {
            foundDup = true;
        }
    }
    else
    {
        if (serviceMap.find(registration.id()) != serviceMap.end())
        {
            Log::warning("Replacing existing provider for service id '%s'", registration.id().c_str()); 
        }
        // Add as service provider, overwriting any existing provider for this service
        serviceMap[registration.id()] = registration.url();
            
        // Remove any previous publisher registrations at this URL as they obviously no longer exist
        purgeObsoletePublishers(registration.id(), registration.url());
    }
    Log::message("[Register] ID: %s, MessageType: %s, URL: %s", registration.id().c_str(),
            registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA ? "Data Product": "Service", registration.url().c_str());


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
    if (unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA)
    {
        list<string>* urls = &dataProductMap[unregistration.id()];
        list<string>::iterator iter = find(urls->begin(), urls->end(), unregistration.url());
        if (iter != urls->end())
        {
            dataProductMap[unregistration.id()].erase(iter);
            GravityDataProduct update(REGISTERED_PUBLISHERS);
            addPublishers(unregistration.id(), update);
            gn.publish(update, unregistration.id());
        }
        else
        {
            foundUrl = false;
        }
    }
    else
    {
        if (serviceMap.find(unregistration.id()) != serviceMap.end())
        {
            serviceMap.erase(unregistration.id());
        }
        else
        {
            foundUrl = false;
        }
    }

    Log::message("[Unregister] ID: %s, MessageType: %s, URL: %s", unregistration.id().c_str(),
            unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA? "Data Product": "Service", unregistration.url().c_str());

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(unregistration.id());
    if (!foundUrl)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::NOT_REGISTERED);
        Log::warning("Attempt to unregister unregistered %s (%s)", unregistration.id().c_str(), unregistration.url().c_str());
    }
    else
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::SUCCESS);
    }

    response.setData(sdr);
}

void ServiceDirectory::purgeObsoletePublishers(const string &dataProductID, const string &url)
{
    for (map<string,list<string> >::iterator iter = dataProductMap.begin(); iter != dataProductMap.end(); iter++)
    {
        if (iter->first != dataProductID)
        {
            list<string>& urls = iter->second;
            list<string>::iterator it = find(urls.begin(), urls.end(), url);
            if (it != urls.end())
            {
                // We need to remove/"unregister" this one
                Log::message("[Auto-Unregister] ID: %s, MessageType: Data Product, URL: %s",
                                iter->first.c_str(), url.c_str());
                urls.erase(it);

                GravityDataProduct update(REGISTERED_PUBLISHERS);
                addPublishers(iter->first, update);
                gn.publish(update, iter->first);
            }
        }
    }
}

void ServiceDirectory::addPublishers(const string &dataProductID, GravityDataProduct &response)
{
    list<string>* urls = &dataProductMap[dataProductID];
    ComponentDataLookupResponsePB lookupResponse;
    lookupResponse.set_lookupid(dataProductID);
    for (list<string>::iterator iter = urls->begin(); iter != urls->end(); iter++)
    {
        lookupResponse.add_url(*iter);
    }
    response.setData(lookupResponse);
}

} /* namespace gravity */
