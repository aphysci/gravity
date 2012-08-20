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

        if (strcmp(requestType.c_str(), "lookup"))
        {
            handleLookup(gdpRequest, gdpResponse);
        }
        else if (strcmp(requestType.c_str(), "register"))
        {
            handleRegister(gdpRequest, gdpResponse);
        }
        else if (strcmp(requestType.c_str(), "unregister"))
        {
            handleUnregister(gdpRequest, gdpResponse);
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
    string url;
    if (lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA)
        url = dataProductMap[lookupRequest.lookupid()];
    else
        url = serviceMap[lookupRequest.lookupid()];

    ComponentLookupResponsePB lookupResponse;
    lookupResponse.set_lookupid(lookupRequest.lookupid());
    lookupResponse.set_url(url);
    response.setData(lookupResponse);
}

void ServiceDirectory::handleRegister(const GravityDataProduct& request, GravityDataProduct& response)
{
    ServiceDirectoryRegistrationPB registration;
    request.populateMessage(registration);
    string currentUrl;
    if (registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA)
    {
        currentUrl = dataProductMap[registration.id()];
        if (currentUrl == "")
            dataProductMap[registration.id()] = registration.url();
    }
    else
    {
        currentUrl = serviceMap[registration.id()];
        if (currentUrl == "")
            serviceMap[registration.id()] = registration.url();
    }

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(registration.id());
    if (currentUrl != "")
    {
        string error;
        if (currentUrl == registration.url())
        {
            sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_DUPLICATE_REGISTRATION);
            error = "Attempt to register duplicate url (" + registration.url() + ") for " + registration.id();
        }
        else
        {
            sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_REGISTRATION_CONFLICT);
            error = "Attempt to register different url (" + registration.url() + ", currently is " + currentUrl
                    + " for " + registration.id();
        }
        cout << error << endl;
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);

    response.setData(sdr);
}

void ServiceDirectory::handleUnregister(const GravityDataProduct& request, GravityDataProduct& response)
{
    ServiceDirectoryUnregistrationPB unregistration;
    request.populateMessage(unregistration);

    string currentUrl;
    if (unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA)
    {
        currentUrl = dataProductMap[unregistration.id()];
        if (currentUrl != "")
            dataProductMap[unregistration.id()] = "";
    }
    else
    {
        currentUrl = serviceMap[unregistration.id()];
        if (currentUrl != "")
            serviceMap[unregistration.id()] = "";
    }

    ServiceDirectoryResponsePB sdr;
    sdr.set_id(unregistration.id());
    if (currentUrl == "")
    {
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_NOT_REGISTERED);
        cout << "Attempt to unregister unregistered " + unregistration.id() << endl;
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);

    response.setData(sdr);
}

} /* namespace gravity */
