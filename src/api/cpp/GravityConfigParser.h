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
