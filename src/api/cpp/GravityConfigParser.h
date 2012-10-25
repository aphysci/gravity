#ifndef GRAVITYCONFIGPARSER_H_
#define GRAVITYCONFIGPARSER_H_

#include "GravityLogger.h"
#include "Utility.h"
#include <iostream>
#include <string>
#include <map>

namespace gravity {

class GravityConfigParser
{
public:
    /** Constructor */
    GravityConfigParser(std::string componentID);
    /** Update configuration based on .ini file */
    void ParseConfigFile(const char* config_filename);
    /** Update configuration based on Config Service */
    void ParseConfigService(GravityNode &gn);

    std::string getString(std::string key, std::string default_value = "");
private:
    std::string componentID;
    std::map<std::string, std::string> key_value_map;

    static int CONFIG_REQUEST_TIMEOUT;
};

}

#endif // GRAVITYCONFIGPARSER_H_
