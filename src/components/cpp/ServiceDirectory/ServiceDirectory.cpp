/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#include "ServiceDirectory.h"
#include "GravityLogger.h"
#include "CommUtil.h"

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

using namespace std;
using namespace gravity;

int main(void)
{
    ServiceDirectory serviceDirectory;
    serviceDirectory.start();
}

static void* registerDataProduct(void* node)
{
    ((GravityNode*)node)->registerDataProduct(REGISTERED_PUBLISHERS, "tcp");
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

	Log::initAndAddConsoleLogger(Log::LogStringToLevel(gn.getStringParam("LocalLogLevel", "warning").c_str()));

    std::string sdURL = gn.getStringParam("ServiceDirectoryUrl", "tcp://*:5555");
    boost::replace_all(sdURL, "localhost", "127.0.0.1");

    Log::message("running with SD connection string: %s", sdURL.c_str());

    void *context = zmq_init(1);
    if (!context)
    {
        Log::critical("Could not create ZeroMQ context, exiting");
        exit(1);
    }

    // Set up the inproc socket to listen for to requests messages from the GravityNode
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

    // register the data product in another thread so we can process request below
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) == 0)
    {
        pthread_t registerThread;
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&registerThread, &attr, registerDataProduct, (void*)&gn);
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

void ServiceDirectory::handleLookup(const GravityDataProduct& request, GravityDataProduct& response)
{
    ComponentLookupRequestPB lookupRequest;
    request.populateMessage(lookupRequest);
    Log::message("[Lookup Request] ID: %s, MessageType: %s, First Server: %s", lookupRequest.lookupid().c_str(),
            lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA? "Data Product": "Service",
			dataProductMap[lookupRequest.lookupid()].size() != 0 ? 
			dataProductMap[lookupRequest.lookupid()].front().c_str(): ""); //NOTE: 0MQ does not have a concept of who the message was sent from so that info is lost.



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
    bool foundDup = false, foundConflict = false;
    string origUrl;
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
            foundDup = true;
    }
    else
    {
        serviceMap[registration.id()] = registration.url();
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
    else if (foundConflict)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::REGISTRATION_CONFLICT);
        Log::warning("Attempt to register conflicting url (%s, was %s) for %s", registration.url().c_str(), origUrl.c_str(), registration.id().c_str());
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB::SUCCESS);

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
            foundUrl = false;
    }
    else
    {
        string origUrl = serviceMap[unregistration.id()];
        if (origUrl != "")
            serviceMap[unregistration.id()] = "";
        else
            foundUrl = false;
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
        sdr.set_returncode(ServiceDirectoryResponsePB::SUCCESS);

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
                gn.publish(update, dataProductID);
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
        lookupResponse.add_url(*iter);
    response.setData(lookupResponse);
}

} /* namespace gravity */
