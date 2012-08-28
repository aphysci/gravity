/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

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
    gravity::ServiceDirectory* serviceDirectory = new gravity::ServiceDirectory();
    serviceDirectory->start();
}

namespace gravity
{

ServiceDirectory::ServiceDirectory()
{
}

ServiceDirectory::~ServiceDirectory()
{
}

void ServiceDirectory::start()
{
    void *context = zmq_init(1);

    //   Socket to talk to clients
    void *socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket, "tcp://*:5555");

    zmq_msg_t request, response, envelope;
    while (1)
    {
        //   Wait for next request
        zmq_msg_init(&envelope);
        cout << "Waiting for lookup request..." << flush;
        zmq_recvmsg(socket, &envelope, 0);
        int size = zmq_msg_size(&envelope);
        string requestType((char*) zmq_msg_data(&envelope), size);
        zmq_msg_close(&envelope);
        cout << "got a " << requestType << " request" << endl;

        zmq_msg_init(&request);
        cout << "Waiting for data..." << flush;
        zmq_recvmsg(socket, &request, 0);
        size = zmq_msg_size(&request);
        char data [size + 1];
        memcpy(data, zmq_msg_data(&request), size);
        zmq_msg_close(&request);
        cout << "received" << endl;

        GravityDataProduct gdpRequest(data, size);
        GravityDataProduct gdpResponse("DataProductRegistrationResponse");

        if (requestType == "lookup")
        {
            handleLookup(gdpRequest, gdpResponse);
        }
        else if (requestType == "register")
        {
            handleRegister(gdpRequest, gdpResponse);
        }
        else if (requestType == "unregister")
        {
            handleUnregister(gdpRequest, gdpResponse);
        }
        else
        {
            cout << "unknown request type: " << requestType << endl;
        }

        // Send reply back to client
        zmq_msg_init_size(&response, gdpResponse.getSize());
        gdpResponse.serializeToArray(zmq_msg_data(&response));
        zmq_sendmsg(socket, &response, 0);
        zmq_msg_close(&response);
    }
}

void ServiceDirectory::handleLookup(const GravityDataProduct& request, GravityDataProduct& response)
{
    ComponentLookupRequestPB lookupRequest;
    request.populateMessage(lookupRequest);
    list<string>* urls;
    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
        urls = &dataProductMap[lookupRequest.lookupid()];
    else
        urls = &serviceMap[lookupRequest.lookupid()];

    ComponentLookupResponsePB lookupResponse;
    lookupResponse.set_lookupid(lookupRequest.lookupid());
    cout << "DEBUG: lookup request: " << lookupRequest.lookupid() << endl;
    for (list<string>::iterator iter = urls->begin(); iter != urls->end(); iter++)
    {
        lookupResponse.add_url(*iter);
        cout << "DEBUG: found url: " << *iter << endl;
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
        if (iter == urls->end())
            serviceMap[registration.id()].push_back(registration.url());
        else
            foundDup = true;
    }

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(registration.id());
    if (foundDup)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_DUPLICATE_REGISTRATION);
        cout << "Attempt to register duplicate url (" << registration.url() << ") for " << registration.id() << endl;
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

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(unregistration.id());
    if (!foundUrl)
    {
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_NOT_REGISTERED);
        cout << "Attempt to unregister unregistered " + unregistration.id() << "(" << unregistration.url() << ")" << endl;
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);

    response.setData(sdr);
}

} /* namespace gravity */
