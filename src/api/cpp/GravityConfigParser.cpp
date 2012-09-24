#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include <iniparser.h>
#include <map>
#include <iostream>


namespace gravity {

GravityConfigParser::GravityConfigParser(const char* config_filename)
{
    if(!Open(config_filename))
    {
        std::cout << "Could not open config file.  Using defaults.  " << std::endl;
        return;
    }

    log_local_level = Log::WARNING;
    log_net_level = Log::WARNING;

    serviceDirectoryUrl = "tcp://*:5555";

    opt = new ez::ezOptionParser();
}

void GravityConfigParser::ParseCmdLine(int argc, const char** argv)
{
    opt->add("warning", false, 1, '\0', "Local Log Level", "--local_log");
    opt->add("warning", false, 1, '\0', "Network Log Level", "--net_log");

    opt->add("", false, 1, '\0', "Service Directory URL", "--sd_url");

//    opt->add("", false, 1, '\0', "Service Directory Transport Type", "--sd_transport");
//    opt->add("", false, 1, '\0', "Service Directory Host", "--sd_host");
//    opt->add("", false, 1, '\0', "Service Directory Port", "--sd_port");

    opt->parse(argc, argv);

    if(opt->get("--local_log")->isSet)
    {
        std::string localloglevstr;
        opt->get("--local_log")->getString(localloglevstr);
        log_local_level = Log::LogStringToLevel(localloglevstr.c_str());
    }

    if(opt->get("--net_log")->isSet)
    {
        std::string netloglevstr;
        opt->get("--net_log")->getString(netloglevstr);
        log_net_level = Log::LogStringToLevel(netloglevstr.c_str());
    }

    if(opt->get("--sd_url")->isSet)
        opt->get("--sd_url")->getString(serviceDirectoryUrl);
}

void GravityConfigParser::ParseConfigFile()
{
    std::string protocol = IniConfigParser::getString("ServiceDirectory:protocol", "tcp");
    std::string interface = IniConfigParser::getString("ServiceDirectory:interface", "*");
    std::string port = IniConfigParser::getString("ServiceDirectory:port", "5555");
    serviceDirectoryUrl = protocol + "://" + interface + ":" + port;

    std::string localloglevstr = IniConfigParser::getString("General:LogLocalLevel", "warning");
    log_local_level = Log::LogStringToLevel(localloglevstr.c_str());

    std::string netloglevstr = IniConfigParser::getString("General:LogNetLevel", "warning");
    log_net_level = Log::LogStringToLevel(netloglevstr.c_str());
}

}
