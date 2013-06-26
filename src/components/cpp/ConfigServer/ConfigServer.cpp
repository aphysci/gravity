#include <GravityNode.h>
#include <GravityConfigParser.h>
#include <KeyValueParserWrap.h>
#include <protobuf/ConfigRequest.pb.h>

#include <iostream>
#include <vector>
#include <map>

using namespace gravity;
using namespace std;

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
    virtual shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    ~ConfigServer() {};
private:
    KeyValueConfigParser &parser;
};

ConfigServer::ConfigServer(KeyValueConfigParser &other_parser) : parser(other_parser)
{
}

shared_ptr<GravityDataProduct> ConfigServer::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
	ConfigRequestPB cfpb;
	dataProduct.populateMessage(cfpb);
    std::vector<const char *> sections;
    
	//Get the parameters we're going to send from the config file.
	std::map<std::string, std::string> key_value_map;
    std::vector<std::string> keys;  
    
	//First send all general parameters
    sections.push_back("general");
	
	//Then override/add to general parameters with specific parameters
	sections.push_back(cfpb.componentid().c_str());
    
    sections.push_back(NULL);
    
    if(!parser.Open("config_file.ini", sections))
	{
		cout << "Critical Error: Could not open config file: config_file.ini" << endl;
		return shared_ptr<GravityDataProduct>(NULL);
	}
    
    keys = parser.GetKeys();
    
	for(std::vector<std::string>::iterator i = keys.begin();
			i != keys.end(); i++)
	{
		std::string value = parser.GetString(*i);
		if(value != "")
			key_value_map[*i] = value;
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
	KeyValueConfigParser parser;
	
	ConfigServer server(parser);

	GravityNode gn;
	gn.init("ConfigServer");

	gn.registerService("ConfigService", GravityTransportTypes::TCP, server);

	gn.waitForExit();
}
