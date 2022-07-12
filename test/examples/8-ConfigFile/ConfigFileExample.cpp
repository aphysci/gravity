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

#include <iostream>
#include <sstream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

int main()
{
	using namespace gravity;

	GravityNode gn;

	//Initialize gravity, giving this node a componentID.
	GravityReturnCode ret = gn.init("ConfigFileExample");
	while (ret != GravityReturnCodes::SUCCESS)
	{
		spdlog::warn("Could not initialize GravityNode, return code was {} retrying...", ret);
	    ret = gn.init("ConfigFileExample");
	}

	// Note that all config keys are case insensitive
    spdlog::info("ServiceDirectoryURL = {}\n", gn.getStringParam( "ServiceDirectoryURL", "Not found" ));
    spdlog::info("LocalLogLevel = {}\n", gn.getStringParam( "LocalLogLevel", "Not Found" ));
    spdlog::info("bin_ms = {}\n", gn.getFloatParam( "bin_ms", 0. ) );
    spdlog::info("bin_us = {}\n", gn.getFloatParam( "bin_us", 0. ) );
    spdlog::info("win_ms = {}\n", gn.getIntParam( "win_ms", 0 ) );
    spdlog::info("na_samples = {}\n", gn.getIntParam( "na_samples", 0 ) );
    spdlog::info("nsamps = {}\n", gn.getIntParam( "nsamps", 0 ) );
    spdlog::info("nsamps_minus = {}\n", gn.getIntParam( "nsamps_minus", 0 ) );
    spdlog::info("operatorstr = {}\n", gn.getStringParam( "operatorstr", "Not found" ));
    spdlog::info("default_value = {}\n", gn.getStringParam( "default_value", "Not found" ));

    spdlog::info("config_server_value = {}\n", gn.getIntParam( "CONFIG_server_value", 0 ));
    spdlog::info("config_server_overridden_value = {}\n", gn.getIntParam( "CONFIG_server_overridden_value", 0 ));
    return 0;
}
