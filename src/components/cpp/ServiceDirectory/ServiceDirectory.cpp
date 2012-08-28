/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#include "GravityLogger.h"
#include "GravityConfigParser.h"
#include "ServiceDirectory.h"
#include "ServiceDirectoryRegistrationPB.pb.h"
#include "ServiceDirectoryUnregistrationPB.pb.h"
#include "ServiceDirectoryResponsePB.pb.h"
#include "ComponentLookupRequestPB.pb.h"
#include "ComponentLookupResponsePB.pb.h"
#include "zmq.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

int main(void)
{
    gravity::GravityConfigParser parser;
    parser.ParseConfigFile("config.ini");

    gravity::Log::init(NULL, "ServiceDirectory.log", 0, parser.getLocalLogLevel(), parser.getNetLogLevel());

    gravity::ServiceDirectory* serviceDirectory = new gravity::ServiceDirectory(parser.getServiceDirectoryUrl().c_str());
    serviceDirectory->start();
}

namespace gravity
{

ServiceDirectory::ServiceDirectory(const char* ba)
{
    bind_address = strdup(ba);
}

ServiceDirectory::~ServiceDirectory()
{
}

void ServiceDirectory::start()
{
    Log::message("Starting Service Directory");

    Log::message("Initializing ZeroMQ");
    void *context = zmq_init(1);

    //   Socket to talk to clients
    void *socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket, bind_address);
    Log::message("Binding to %s", bind_address);
    free(bind_address); //We don't need this guy any more.
    bind_address = NULL;

    zmq_msg_t request, response, envelope;
    while (1)
    {
        //   Wait for next request
        zmq_msg_init(&envelope);
        gravity::Log::debug("Waiting for lookup request...");
        zmq_recvmsg(socket, &envelope, 0);
        Log::debug("received message");
        int size = zmq_msg_size(&envelope);
        string requestType((char*) zmq_msg_data(&envelope), size);
        zmq_msg_close(&envelope);
        Log::debug("got a %s request", requestType.c_str());

        zmq_msg_init(&request);
        Log::debug("Waiting for data...");
        zmq_recvmsg(socket, &request, 0);
        size = zmq_msg_size(&request);
        char data [size + 1];
        memcpy(data, zmq_msg_data(&request), size);
        zmq_msg_close(&request);
        Log::debug("received");

        Log::debug("Setting up data products from data. ");
        GravityDataProduct gdpRequest(data, size);
        GravityDataProduct gdpResponse("DataProductRegistrationResponse");

        if (requestType == "lookup")
        {
            Log::trace("Handling lookup");
            handleLookup(gdpRequest, gdpResponse);
        }
        else if (requestType == "register")
        {
            Log::trace("Handling register");
            handleRegister(gdpRequest, gdpResponse);
        }
        else if (requestType == "unregister")
        {
            Log::trace("Handling unregister");
            handleUnregister(gdpRequest, gdpResponse);
        }

        // Send reply back to client
        Log::debug("Sending Response");
        zmq_msg_init_size(&response, gdpResponse.getSize());
        Log::trace("Serializign Message");
        gdpResponse.serializeToArray(zmq_msg_data(&response));
        Log::trace("Sending");
        zmq_sendmsg(socket, &response, 0);
        zmq_msg_close(&response);
    }
}

void ServiceDirectory::handleLookup(const GravityDataProduct& request, GravityDataProduct& response)
{
    ComponentLookupRequestPB lookupRequest;
    request.populateMessage(lookupRequest);
    Log::message("[Lookup Request] ID: %s, MessageType: %s", lookupRequest.lookupid().c_str(),
            lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA? "Data Product": "Service"); //NOTE: 0MQ does not have a concept of who the message was sent from so that info is lost.

    list<string>* urls;
    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
        urls = &dataProductMap[lookupRequest.lookupid()];
    else
        urls = &serviceMap[lookupRequest.lookupid()];

    ComponentLookupResponsePB lookupResponse;
    lookupResponse.set_lookupid(lookupRequest.lookupid());
    for (list<string>::iterator iter = urls->begin(); iter != urls->end(); iter++)
    {
        lookupResponse.add_url(*iter);
        Log::debug("Found url: %s", (*iter).c_str());
    }

    response.setData(lookupResponse);
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
            dataProductMap[registration.id()].push_back(registration.url());
        else
            foundDup = true;
    }
    else
    {
        list<string>* urls = &serviceMap[registration.id()];
        list<string>::iterator iter = find(urls->begin(), urls->end(), registration.url());
        if (iter != urls->end())
            serviceMap[registration.id()].push_back(registration.url());
        else
            foundDup = true;
    }
    Log::message("[Register] ID: %s, MessageType: %s, URL: %s", registration.id().c_str(),
            registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA? "Data Product": "Service", registration.url().c_str());


    ServiceDirectoryResponsePB sdr;
    sdr.set_id(registration.id());
    if (foundDup)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_DUPLICATE_REGISTRATION);
        Log::warning("Attempt to register duplicate url (%s) for %s", registration.url().c_str(), registration.id().c_str());
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);

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
        list<string>* urls = &serviceMap[unregistration.id()];
        list<string>::iterator iter = find(urls->begin(), urls->end(), unregistration.url());
        if (iter != urls->end())
            serviceMap[unregistration.id()].erase(iter);
        else
            foundUrl = false;
    }

    Log::message("[Unregister] ID: %s, MessageType: %s, URL: %s", unregistration.id().c_str(),
            unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA? "Data Product": "Service", unregistration.url().c_str());

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(unregistration.id());
    if (!foundUrl)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_NOT_REGISTERED);
        Log::warning("Attempt to unregister unregistered %s (%s)", unregistration.id().c_str(), unregistration.url().c_str());
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);

    response.setData(sdr);
}

} /* namespace gravity */
