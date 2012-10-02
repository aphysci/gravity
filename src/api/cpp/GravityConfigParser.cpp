#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include "GravitySemaphore.h"
#include "protobuf/ConfigRequest.pb.h"

#include <iniparser.h>
#include <map>
#include <iostream>


namespace gravity {

GravityConfigParser::GravityConfigParser(std::string componentID)
{
	this->componentID = componentID;
}

void GravityConfigParser::ParseConfigFile(const char* config_filename)
{
	IniConfigParser parser;

	if(!parser.Open(config_filename)) //TODO: if this doesn't work try stat.
        return; //Fail Silently

	std::vector<std::string> keys = parser.GetSectionKeys("general");

	for(std::vector<std::string>::iterator i = keys.begin();
			i != keys.end(); i++)
	{
		std::string value = parser.getString(*i);
		std::string key_nosection = i->substr(i->find_first_of(":") + 1);
		if(value != "")
			key_value_map[key_nosection] = value;
	}


	keys = parser.GetSectionKeys(componentID);

	for(std::vector<std::string>::iterator i = keys.begin();
			i != keys.end(); i++)
	{
		std::string value = parser.getString(*i);
		std::string key_nosection = i->substr(i->find_first_of(":") + 1);
		if(value != "")
			key_value_map[key_nosection] = value;
	}

	return;
}

class ConfigRequestor: public GravityRequestor
{
public:
	ConfigRequestor(GravityConfigParser* parser, GravityNode &gn);

	void requestFilled(string serviceID, string requestID, const GravityDataProduct& response);
	virtual ~ConfigRequestor() {}

	std::map<std::string, std::string> config_map;
	void WaitForConfig();
private:
	GravityNode &gn;
	GravityConfigParser* parser;
	Semaphore lock;
};

ConfigRequestor::ConfigRequestor(GravityConfigParser* other_parser, GravityNode &other_gn) : gn(other_gn), parser(other_parser),
		lock(0) //Initialize the lock with a value of 0 (locked)
{
}

void ConfigRequestor::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
	ConfigeResponsePB responseMessage;
	response.populateMessage(responseMessage);

	int config_len = min(responseMessage.key_size(), responseMessage.value_size());

	for(int i = 0; i < config_len; i++)
	{
		//if(parser->key_value_map.find(responseMessage.key(i)) == parser->key_value_map.end()) //Don't overwrite keys.
			parser->key_value_map[responseMessage.key(i)] = responseMessage.value(i);
	}

	lock.Unlock();
}

void ConfigRequestor::WaitForConfig()
{
	//Barrier Synchronization.
	lock.Lock();
}

void GravityConfigParser::ParseConfigService(GravityNode &gn)
{
	GravityDataProduct dataproduct("ConfigRequestPB");

	ConfigRequestPB crpb;
	crpb.set_componentid(componentID);

	dataproduct.setData(crpb);

	ConfigRequestor requestor(this, gn);

	gn.request("ConfigService", dataproduct, requestor, componentID);

	//Wait for all values to come in...
	requestor.WaitForConfig();
}

std::string GravityConfigParser::getString(std::string key, std::string default_value)
{
	std::string key_lower = StringCopyToLowerCase(key);
	std::map<std::string, std::string>::iterator i = key_value_map.find(key_lower);
	if(i == key_value_map.end())
		return default_value;
	else
		return i->second;
}

}
