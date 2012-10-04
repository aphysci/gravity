#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include "GravitySemaphore.h"
#include "protobuf/ConfigRequest.pb.h"

#include <iniparser.h>
#include <map>
#include <iostream>


namespace gravity {

int GravityConfigParser::CONFIG_REQUEST_TIMEOUT = 4000;

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


void GravityConfigParser::ParseConfigService(GravityNode &gn)
{
	//Prepare request
	GravityDataProduct dataproduct("ConfigRequestPB");
	ConfigRequestPB crpb;
	crpb.set_componentid(componentID);
	dataproduct.setData(crpb);

	//Send Request/Get Response
	shared_ptr<GravityDataProduct> response = gn.request("ConfigService", dataproduct, CONFIG_REQUEST_TIMEOUT);
	if(response == NULL)
		return;

	ConfigeResponsePB responseMessage;
	response->populateMessage(responseMessage);

	//Parse Response
	int config_len = min(responseMessage.key_size(), responseMessage.value_size());
	for(int i = 0; i < config_len; i++)
	{
		//if(key_value_map.find(responseMessage.key(i)) == key_value_map.end()) //Don't overwrite keys.
			key_value_map[responseMessage.key(i)] = responseMessage.value(i);
	}
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
