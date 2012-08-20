/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#include "ServiceDirectory.h"
#include "ServiceDirectoryRegistrationPB.pb.h"
#include "ServiceDirectoryResponsePB.pb.h"
#include "ComponentLookupRequestPB.pb.h"
#include "ComponentLookupResponsePB.pb.h"
#include "zmq.h"

#include <glib.h>

#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

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
	g_message("Starting Service Directory");

	g_message("Initializing ZeroMQ");
    void *context = zmq_init(1);
    //   Socket to talk to clients
    void *socket = zmq_socket(context, ZMQ_REP);

    g_message("Parsing Config File");

    GKeyFile* keyFile = g_key_file_new();

    if(!g_key_file_load_from_file(keyFile, "ServiceDirectory.ini", G_KEY_FILE_NONE, NULL))
		g_error("Couldn't Load ServiceDirectory.ini.  I don't know where to bind!!!");

	gchar* sdprotocol = g_key_file_get_string(keyFile, "ServiceDirectory", "protocol", NULL);

	if(!sdprotocol)
	{
		g_warning("No protocol specified.  Assuming tcp.");
		sdprotocol = g_strdup("tcp");
	}

	gchar* sdinterface = g_key_file_get_string(keyFile, "ServiceDirectory", "interface", NULL);

	if(!sdinterface)
		sdinterface = g_strdup("*");

	gchar* sdport = g_key_file_get_string(keyFile, "ServiceDirectory", "port", NULL);

	if(!sdport)
		g_error("No port specified in config file.");


	stringstream url;
	url << sdprotocol << "//" << sdinterface << ":" << sdport << "\0";

    g_message("Binding to %s", url.str().c_str());
    zmq_bind(socket, url.str().c_str());

    zmq_msg_t request, response, envelope;
    while (1)
    {
        //   Wait for next request
        zmq_msg_init(&envelope);
        g_message("Waiting for lookup request...");
        zmq_recvmsg(socket, &envelope, 0);
        g_debug("received message");
        int size = zmq_msg_size(&envelope);
        string* requestType = new string((char*) zmq_msg_data(&envelope), size);
        zmq_msg_close(&envelope);
  		g_message("got a %s request", requestType);

        zmq_msg_init(&request);
        g_message("Waiting for data...");
        zmq_recvmsg(socket, &request, 0);
        size = zmq_msg_size(&request);
        void* data = (char*) malloc(size + 1);
        memcpy(data, zmq_msg_data(&request), size);
        zmq_msg_close(&request);
        g_message("received");

        GravityDataProduct gdpRequest(data, size);
        GravityDataProduct gdpResponse("DataProductRegistrationResponse");

        if (strcmp(requestType->c_str(), "lookup"))
        {
            handleLookup(gdpRequest, gdpResponse);
        }
        else if (strcmp(requestType->c_str(), "register"))
        {
            handleRegister(gdpRequest, gdpResponse);
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
    {
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);
    }

    response.setData(sdr);
}

} /* namespace gravity */
