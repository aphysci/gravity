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

#include <glib.h>

#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN    ((gchar*) "ServiceDirectory")
static GLogLevelFlags current_log_level;

void               my_log_default_handler               (const gchar *log_domain,
                                                         GLogLevelFlags log_level,
                                                         const gchar *message,
                                                         gpointer unused_data)
{
	//Format the Logs nicely.
	char timestr[100];
	time_t rawtime;
	struct tm * timeinfo;

	if(!(current_log_level & log_level))
		return;

	time ( &rawtime );
	timeinfo = localtime( &rawtime );

	strftime(timestr, 100, "%m/%d/%y %H:%M:%S", timeinfo);

	cout << "[";
	if(log_domain != NULL)
		cout << log_domain << " ";
	cout << timestr << " ";

	if(log_level & G_LOG_LEVEL_ERROR)
		cout << "ERROR";
	else if(log_level & G_LOG_LEVEL_CRITICAL)
		cout << "CRITICAL";
	else if(log_level & G_LOG_LEVEL_WARNING)
		cout << "WARNING";
	else if(log_level & G_LOG_LEVEL_MESSAGE)
		cout << "MESSAGE";
	else if(log_level & G_LOG_LEVEL_INFO)
		cout << "INFO";
	else if(log_level & G_LOG_LEVEL_DEBUG)
		cout << "DEBUG";
	cout << "] ";

	cout << message << endl;
}


static gchar* g_config_file = NULL;
static bool config_done = false;
gboolean            ParseLogLevelOption                 (const gchar *option_name, //Not Used
                                                         const gchar *value,
                                                         gpointer data, //Not Used
                                                         GError **gerror)
{
	if(config_done)
		return TRUE;
	config_done = true;//Only let config be done once.. ie command line overrides config file.

	gchar *sdloglevel = strdup(value);

	std::use_facet< std::ctype<char> >(std::locale("")).tolower(&sdloglevel[0], &sdloglevel[0] + strlen(sdloglevel)); //Convert to lowercase.
	if(strcmp(sdloglevel, "debug") == 0)
		current_log_level = (GLogLevelFlags)(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG);
	else if(strcmp(sdloglevel, "info") == 0)
		current_log_level = (GLogLevelFlags)(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO);
	else if(strcmp(sdloglevel, "message") == 0)
		current_log_level = (GLogLevelFlags)(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE);
	else if(strcmp(sdloglevel, "warning") == 0)
		current_log_level = (GLogLevelFlags)(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
	else if(strcmp(sdloglevel, "critical") == 0)
		current_log_level = (GLogLevelFlags)(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
	else if(strcmp(sdloglevel, "error") == 0)
		current_log_level = (GLogLevelFlags) G_LOG_LEVEL_ERROR;
	else
	{
		g_free(sdloglevel);
		const gchar* invalidmsg = "Invalid Log Level";
		g_set_error(gerror, g_quark_from_string(invalidmsg), (gint) 1, "%s", invalidmsg);
		return FALSE;
	}

	g_free(sdloglevel);
	return TRUE;
}

static GOptionEntry entries[] =
{
  { "config-file", 'c', G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME, &g_config_file, "Configuration Filename", "C" },
  { "log-level", 'd', 0, G_OPTION_ARG_CALLBACK, (void*)&ParseLogLevelOption, "Log Level Options", "D" },
  { NULL }
};

int main(int argc, char** argv)
{
	current_log_level = (GLogLevelFlags)(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG);

	//Parse Command Line
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new ("- Service Directory Process");
	g_option_context_add_main_entries (context, entries, NULL);
	if(!g_option_context_parse (context, &argc, &argv, &error))
	{
		g_print ("option parsing failed: %s\n", error->message);
		exit(1);
	}

	if(g_config_file == NULL)
		g_config_file = "ServiceDirectory.ini";

	//Start Service Directory.
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

	g_message("Parsing Config File");

    GKeyFile* keyFile = g_key_file_new();

    if(!g_key_file_load_from_file(keyFile, "ServiceDirectory.ini", G_KEY_FILE_NONE, NULL))
		g_error("Couldn't Load config file.  I don't know where to bind!!!");

	//Initialize Logging Level
	gchar* sdloglevel = g_key_file_get_string(keyFile, "ServiceDirectory", "loglevel", NULL);
	if(sdloglevel)
	{
		GError* error = NULL;
		ParseLogLevelOption(NULL, sdloglevel, NULL, &error);
		g_error_free(error);
	}

    //Get URL
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
	g_free(sdprotocol);
	g_free(sdinterface);
	g_free(sdport);

	g_key_file_free(keyFile);

	g_message("Initializing ZeroMQ");
    void *context = zmq_init(1);
    //   Socket to talk to clients
    void *socket = zmq_socket(context, ZMQ_REP);

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
        string requestType((char*) zmq_msg_data(&envelope), size);
        zmq_msg_close(&envelope);
  		g_message("got a %s request", requestType.c_str());

        zmq_msg_init(&request);
        g_message("Waiting for data...");
        zmq_recvmsg(socket, &request, 0);
        size = zmq_msg_size(&request);
        char data [size + 1];
        memcpy(data, zmq_msg_data(&request), size);
        zmq_msg_close(&request);

        g_message("Setting up data products from data. ");
        GravityDataProduct gdpRequest(data, size);
        GravityDataProduct gdpResponse("DataProductRegistrationResponse");

        if (strcmp(requestType.c_str(), "lookup"))
        {
        	g_message("Handling lookup");
            handleLookup(gdpRequest, gdpResponse);
        }
        else if (strcmp(requestType.c_str(), "register"))
        {
        	g_message("Handling register");
            handleRegister(gdpRequest, gdpResponse);
        }
        else if (strcmp(requestType.c_str(), "unregister"))
        {
        	g_message("Handling unregister");
            handleUnregister(gdpRequest, gdpResponse);
        }
        else
        	g_warning("Unknown message %s", requestType.c_str());

        // Send reply back to client
        g_message("Sending Response");
        zmq_msg_init_size(&response, gdpResponse.getSize());
        g_debug("Serializign Message");
        gdpResponse.serializeToArray(zmq_msg_data(&response));
        g_debug("Sending");
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

    g_debug("ID: %s, MessageType: %s, URL: %s", lookupRequest.lookupid().c_str(),
    		lookupRequest.type() == ComponentLookupRequestPB_RegistrationType_DATA? "Data Product": "Service", url.c_str());

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
        g_warning("%s", error.c_str());
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);
    g_debug("ID: %s, MessageType: %s, URL: %s", registration.id().c_str(),
    		registration.type() == ServiceDirectoryRegistrationPB_RegistrationType_DATA? "Data Product": "Service", registration.url().c_str());

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
        g_warning("Attempt to unregister unregistered %s", unregistration.id().c_str());
    }
    else
        sdr.set_returncode(ServiceDirectoryResponsePB_ReturnCodes_SUCCESS);

    g_debug("ID: %s, MessageType: %s, URL: %s", unregistration.id().c_str(),
    		unregistration.type() == ServiceDirectoryUnregistrationPB_RegistrationType_DATA? "Data Product": "Service", currentUrl.c_str());

    response.setData(sdr);
}

} /* namespace gravity */
