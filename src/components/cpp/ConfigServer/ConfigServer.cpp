#include <GravityNode.h>
#include <GravityConfigParser.h>
#include <protobuf/ConfigRequest.pb.h>

#include <vector>
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

    //map<requestor, vector<section, key, value> >;
    std::map<std::string, std::vector<ConfigEntry> > config_map; //For recording configs.
};

ConfigServer::ConfigServer(IniConfigParser &other_parser) : parser(other_parser)
{
}

shared_ptr<GravityDataProduct> ConfigServer::request(const GravityDataProduct& dataProduct)
{
	cout << "serviceID: " << dataProduct.getDataProductID() << endl;

	ConfigRequestPB cfpb;
	dataProduct.populateMessage(cfpb);

	std::string value = parser.getString(cfpb.section(), cfpb.key());

	ConfigEntry ce(cfpb.section(), cfpb.key(), value);
	config_map[cfpb.requestor()].push_back(ce);

	ConfigeResponsePB message;
	message.set_response(value);
	shared_ptr<GravityDataProduct> response(new GravityDataProduct("ConfigResponsePB"));
	response->setData(message);

	if(cfpb.has_start() && cfpb.start() == true)
	{
		//TODO: Write to Log file.
	}

	return response;
}

int main(int argc, const char** argv)
{
	IniConfigParser parser;
	parser.Open("config_file.ini");

	ConfigServer server(parser);

	GravityNode gn;
	gn.init();

	gn.registerService("ConfigService", 54542, "tcp", server);

	while(true)
		Sleep(2000000000);
}
