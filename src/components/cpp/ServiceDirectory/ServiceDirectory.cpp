/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#include "ServiceDirectory.h"
#include "GravityLogger.h"

#include "protobuf/ServiceDirectoryRegistrationPB.pb.h"
#include "protobuf/ServiceDirectoryUnregistrationPB.pb.h"
#include "protobuf/ServiceDirectoryResponsePB.pb.h"
#include "protobuf/ComponentLookupRequestPB.pb.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ComponentServiceLookupResponsePB.pb.h"

#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace gravity;

int main(void)
{
	GravityNode gn;
    Log::initAndAddConsoleLogger(Log::MESSAGE);
	gn.init("ServiceDirectory");

    std::string sdURL = gn.getStringParam("ServiceDirectoryUrl", "tcp://*:5555");

    Log::message("running with SD connection string: %s", sdURL.c_str());
    ServiceDirectory serviceDirectory;
	gn.registerService("ServiceDirectory", sdURL, serviceDirectory, false);

	gn.waitForExit();
}

namespace gravity
{

ServiceDirectory::~ServiceDirectory()
{
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
    Log::message("[Lookup Request] ID: %s, MessageType: %s", lookupRequest.lookupid().c_str(),
            lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA? "Data Product": "Service"); //NOTE: 0MQ does not have a concept of who the message was sent from so that info is lost.

    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
    {
        list<string>* urls = &dataProductMap[lookupRequest.lookupid()];
        ComponentDataLookupResponsePB lookupResponse;
        lookupResponse.set_lookupid(lookupRequest.lookupid());
        for (list<string>::iterator iter = urls->begin(); iter != urls->end(); iter++)
        {
            lookupResponse.add_url(*iter);
            Log::debug("Found url: %s", (*iter).c_str());
        }
        response.setData(lookupResponse);
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
    bool foundDup = false, foundConflict = false;;
    string origUrl;
    if (registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA)
    {
        list<string>* urls = &dataProductMap[registration.id()];
        list<string>::iterator iter = find(urls->begin(), urls->end(), registration.url());
        if (iter == urls->end())
            dataProductMap[registration.id()].push_back(registration.url());
        else
            foundDup = true;
    }
    else
    {
        origUrl = serviceMap[registration.id()];
        if (origUrl == "")
            serviceMap[registration.id()] = registration.url();
        else if (origUrl == registration.url())
            foundConflict = true;
        else
            foundDup = true;
    }
    Log::message("[Register] ID: %s, MessageType: %s, URL: %s", registration.id().c_str(),
            registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA? "Data Product": "Service", registration.url().c_str());


    ServiceDirectoryResponsePB sdr;
    sdr.set_id(registration.id());
    if (foundDup)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB::DUPLICATE_REGISTRATION);
        Log::warning("Attempt to register duplicate url (%s) for %s", registration.url().c_str(), registration.id().c_str());
    }
    else if (foundConflict)
    {
    	//TODO: check if registration.url() == origUrl
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
            dataProductMap[unregistration.id()].erase(iter);
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

} /* namespace gravity */
