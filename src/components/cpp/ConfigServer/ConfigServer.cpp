#include <GravityNode.h>
#include <GravityConfigParser.h>
#include <protobuf/ConfigRequest.pb.h>

#include <iostream>
#include <vector>
#include <map>

using namespace gravity;

struct ConfigEntry
{
	ConfigEntry(std::string s1, std::string s2, std::string s3)
	{
		section = s1;
		key = s2;
		value = s3;
	}
	std::string section;
	std::string key;
	std::string value;
};

class ConfigServer : public GravityServiceProvider
{
public:
	ConfigServer(IniConfigParser &parser);
    virtual shared_ptr<GravityDataProduct> request(const GravityDataProduct& dataProduct);
    ~ConfigServer() {};
private:
    IniConfigParser &parser;
};

ConfigServer::ConfigServer(IniConfigParser &other_parser) : parser(other_parser)
{
}

shared_ptr<GravityDataProduct> ConfigServer::request(const GravityDataProduct& dataProduct)
{
	ConfigRequestPB cfpb;
	dataProduct.populateMessage(cfpb);

	//Get the parameters we're going to send from the config file.
	std::map<std::string, std::string> key_value_map;

	//First send all general parameters
	std::vector<std::string> keys = parser.GetSectionKeys("General");

	for(std::vector<std::string>::iterator i = keys.begin();
			i != keys.end(); i++)
	{
		std::string value = parser.getString(*i);
		std::string key_nosection = i->substr(i->find_first_of(":") + 1);
		if(value != "")
			key_value_map[key_nosection] = value;
	}

	//Then override/add to general parameters with specific parameters
	keys = parser.GetSectionKeys(cfpb.componentid().c_str());
	for(std::vector<std::string>::iterator i = keys.begin();
			i != keys.end(); i++)
	{
		std::string value = parser.getString(*i);
		std::string key_nosection = i->substr(i->find_first_of(":") + 1);
		if(value != "")
			key_value_map[key_nosection] = value;
	}

	//Populate Response Message and Send it
	Log::message("Sending Config to %s", cfpb.componentid().c_str());
	ConfigeResponsePB message;
	for(std::map<std::string, std::string>::iterator i = key_value_map.begin();
			i != key_value_map.end(); i++)
	{
		Log::message("\t%s=%s", i->first.c_str(), i->second.c_str());
		message.add_key(i->first);
		message.add_value(i->second);
	}

	shared_ptr<GravityDataProduct> response(new GravityDataProduct("ConfigResponsePB"));
	response->setData(message);

	return response; //Returing the message will send it.
}

int main(int argc, const char** argv)
{
	IniConfigParser parser;
	if(!parser.Open("config_file.ini"))
	{
		cout << "Critical Error: Could not open config file: config_file.ini" << endl;
		return -1;
	}
	ConfigServer server(parser);

	GravityNode gn;
	gn.init("ConfigServer");

	gn.registerService("ConfigService", 54542, "tcp", server);

	while(true)
		Sleep(2000000000);
}
