/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

#include <GravityNode.h>
#include <GravityConfigParser.h>
#include <KeyValueParserWrap.h>
#include <SpdLog.h>
#include <protobuf/ConfigRequest.pb.h>

#include <iostream>
#include <vector>
#include <map>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"

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
	ConfigServer() {}
    virtual std::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    ~ConfigServer() {};
	
};

std::shared_ptr<GravityDataProduct> ConfigServer::request(const std::string serviceID, const GravityDataProduct& dataProduct)
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

    KeyValueConfigParser parser("config_file.ini", sections);

    keys = parser.GetKeys();

	for(std::vector<std::string>::iterator i = keys.begin();
			i != keys.end(); i++)
	{
		std::string value = parser.GetString(*i);
		if(value != "")
			key_value_map[*i] = value;
	}

    if(!key_value_map.size())
	{
		cout << "Critical Error: Could not open config file: config_file.ini" << endl;
		return std::shared_ptr<GravityDataProduct>();
	}

	//Populate Response Message and Send it
	SpdLog::info(fmt::format("Sending Config to {}", cfpb.componentid()).c_str());
	ConfigeResponsePB message;
	for(std::map<std::string, std::string>::iterator i = key_value_map.begin();
			i != key_value_map.end(); i++)
	{
		SpdLog::info(fmt::format("\t{}={}", i->first, i->second).c_str());
		message.add_key(i->first);
		message.add_value(i->second);
	}

	std::shared_ptr<GravityDataProduct> response(new GravityDataProduct("ConfigResponsePB"));
	response->setData(message);

	return response; //Returning the message will send it.
}

int main(int argc, const char** argv)
{
	ConfigServer server;

	GravityNode gn;
	GravityReturnCode ret = gn.init("ConfigServer");
	while (ret != GravityReturnCodes::SUCCESS)
	{
	    cerr << "Failed to initialize ConfigServer, retrying..." << endl;
	    ret = gn.init("ConfigServer");
	}

	gn.registerService("ConfigService", GravityTransportTypes::TCP, server);

	gn.waitForExit();
}
