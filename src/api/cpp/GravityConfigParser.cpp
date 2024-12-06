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

#include "GravityConfigParser.h"
#include "GravityLogger.h"
#include "GravitySemaphore.h"
#include "protobuf/ConfigRequest.pb.h"
#include "KeyValueParserWrap.h"

#include <memory>
#include <map>
#include <iostream>

#ifdef min
#undef min
#endif

namespace gravity
{

int GravityConfigParser::CONFIG_REQUEST_TIMEOUT = 4000;

bool GravityConfigParser::hasKey(std::string key) { return key_value_map.count(StringCopyToLowerCase(key)) > 0; }

GravityConfigParser::GravityConfigParser(std::string componentID) { this->componentID = componentID; }

void GravityConfigParser::ParseConfigFile(const char *config_filename)
{
    std::vector<const char *> sections;

    sections.push_back(componentID.c_str());
    sections.push_back("general");
    sections.push_back(NULL);

    KeyValueConfigParser parser(config_filename, sections);

    std::vector<std::string> keys = parser.GetKeys();

    for (std::vector<std::string>::iterator i = keys.begin(); i != keys.end(); i++)
    {
        std::string value = parser.GetString(*i);
        std::string key_lower = StringCopyToLowerCase(*i);
        key_value_map[key_lower] = value;
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
    std::shared_ptr<GravityDataProduct> response = gn.request("ConfigService", dataproduct, CONFIG_REQUEST_TIMEOUT);
    if (response == NULL) return;

    ConfigeResponsePB responseMessage;
    response->populateMessage(responseMessage);

    //Parse Response
    int config_len = std::min(responseMessage.key_size(), responseMessage.value_size());
    for (int i = 0; i < config_len; i++)
    {
        std::string key_lower = StringCopyToLowerCase(responseMessage.key(i));
        if (key_value_map.find(key_lower) == key_value_map.end())  //Don't overwrite keys.
            key_value_map[key_lower] = responseMessage.value(i);
    }
}

std::string GravityConfigParser::getString(std::string key, std::string default_value)
{
    std::string key_lower = StringCopyToLowerCase(key);
    std::map<std::string, std::string>::iterator i = key_value_map.find(key_lower);
    if (i == key_value_map.end())
        return default_value;
    else
        return i->second;
}

}  // namespace gravity
