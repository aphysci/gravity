#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include <iniparser.h>
#include <map>
#include <iostream>

namespace gravity {

GravityConfigParser::GravityConfigParser()
{
    serviceDirectoryUrl = NULL;
}

GravityConfigParser::~GravityConfigParser()
{
    free(serviceDirectoryUrl);
}


void GravityConfigParser::ParseConfigFile(const char* filename, const char** additional_keys_to_extract)
{
    dictionary* myconfig = iniparser_load(filename);

    if(!myconfig)
    {
        std::cout << "Could not open config file.  Using defaults.  " << std::endl;
        log_local_level = Log::WARNING;
        log_net_level = Log::WARNING;

        if(!serviceDirectoryUrl)
            serviceDirectoryUrl = strdup("tcp://*:5555");

        return;
    }

    //Ini parser causes warnings!!!
    const char* protocol = iniparser_getstring(myconfig, "ServiceDirectory:protocol", "tcp");
    const char* interface = iniparser_getstring(myconfig, "ServiceDirectory:interface", "*");
    const char* port = iniparser_getstring(myconfig, "ServiceDirectory:port", "5555");

    free(serviceDirectoryUrl);
    serviceDirectoryUrl = (char*) malloc(sizeof(char)*256);

    sprintf(serviceDirectoryUrl, "%s://%s:%s", protocol, interface, port);

    const char* loglevstr = iniparser_getstring(myconfig, "General:LogLocalLevel", "warning");
    log_local_level = Log::LogStringToLevel(loglevstr);

    loglevstr = iniparser_getstring(myconfig, "General:LogNetLevel", "warning");
    log_net_level = Log::LogStringToLevel(loglevstr);

    while(additional_keys_to_extract != NULL)
    {
        values[*additional_keys_to_extract] = string(iniparser_getstring(myconfig, (*additional_keys_to_extract), ""));
        additional_keys_to_extract++;
    }

    GetOtherConfigOptions(myconfig);

    iniparser_freedict(myconfig);
}

void GravityConfigParser::GetOtherConfigOptions(dictionary* myconfig)
{

}

}
