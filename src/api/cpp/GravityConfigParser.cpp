#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include "GravitySemaphore.h"
#include "protobuf/ConfigRequest.pb.h"

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
    std::string my_interface = IniConfigParser::getString("ServiceDirectory:interface", "*");
    std::string port = IniConfigParser::getString("ServiceDirectory:port", "5555");
    serviceDirectoryUrl = protocol + "://" + my_interface + ":" + port;

    std::string localloglevstr = IniConfigParser::getString("General:LogLocalLevel", "warning");
    log_local_level = Log::LogStringToLevel(localloglevstr.c_str());

    std::string netloglevstr = IniConfigParser::getString("General:LogNetLevel", "warning");
    log_net_level = Log::LogStringToLevel(netloglevstr.c_str());
}

class ConfigRequestor: public GravityRequestor
{
public:
	ConfigRequestor(GravityNode &gn);
	void gravityConfigRequest(string componentID, string section, string key, bool start = false);

	void requestFilled(string serviceID, string requestID, const GravityDataProduct& response);
	virtual ~ConfigRequestor() {}

	std::map<std::string, std::string> config_map;
	void WaitForConfig();
private:
	GravityNode &gn;
	Semaphore lock;
	int nConfigs;
};

ConfigRequestor::ConfigRequestor(GravityNode &other_gn) : gn(other_gn)
{
	nConfigs = 0;
}

void ConfigRequestor::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
	cout << "serviceID: " << serviceID << endl;
	cout << "requestID: " << requestID << endl;
	cout << "dataProductID: " << response.getDataProductID() << endl;
	cout << "dataSize: " << response.getDataSize() << endl;

	ConfigeResponsePB responseMessage;
	response.populateMessage(responseMessage);
	config_map[requestID] = responseMessage.response();

	lock.Unlock();
}

void ConfigRequestor::gravityConfigRequest(string componentID, string section, string key, bool start)
{
	nConfigs++;
	ConfigRequestPB crpb;
	crpb.set_requestor(componentID);
	crpb.set_section(section);
	crpb.set_key(key);
	if(start)
		crpb.set_start(true);

	GravityDataProduct dataproduct("ConfigRequestPB");
	dataproduct.setData(crpb);

	string sectionkey = section + ":" + key;
	gn.request("ConfigService", dataproduct, *this, sectionkey);
}

void ConfigRequestor::WaitForConfig()
{
	for(; nConfigs >= 0; nConfigs--)
		lock.Lock();
}

void GravityConfigParser::ParseConfigService(GravityNode &gn, std::string componentID)
{
	ConfigRequestor requestor(gn);
	requestor.gravityConfigRequest(componentID, "General", "LogLocalLevel");
	requestor.gravityConfigRequest(componentID, "General", "LogNetLevel");

	//Wait for all values to come in...
	requestor.WaitForConfig();

    log_local_level = Log::LogStringToLevel(requestor.config_map["General:LogLocalLevel"].c_str());
    log_net_level = Log::LogStringToLevel(requestor.config_map["General:LogLocalLevel"].c_str());
}

}
