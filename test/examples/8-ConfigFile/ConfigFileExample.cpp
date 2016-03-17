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
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::fatal("Could not initialize GravityNode, return code was %d", ret);
		exit(1);
	}

    Log::message("ServiceDirectoryURL = %s\n", gn.getStringParam( "ServiceDirectoryURL", "Not found" ).c_str() );
    Log::message("ConfigFileLogLevel = %s\n", gn.getStringParam( "LocalLogLevel", "Not Found" ).c_str() );
    Log::message("bin_ms = %f\n", gn.getFloatParam( "bin_ms", 0. ) );
    Log::message("bin_us = %f\n", gn.getFloatParam( "bin_us", 0. ) );
    Log::message("win_ms = %d\n", gn.getIntParam( "win_ms", 0 ) );
    Log::message("na_samples = %d\n", gn.getIntParam( "na_samples", 0 ) );
    Log::message("nsamps = %d\n", gn.getIntParam( "nsamps", 0 ) );
    Log::message("nsamps_minus = %d\n", gn.getIntParam( "nsamps_minus", 0 ) );
    Log::message("operatorstr = %s\n", gn.getStringParam( "operatorstr", "Not found" ).c_str() );
    Log::message("default_value = %s\n", gn.getStringParam( "default_value", "Not found" ).c_str() );
    return 0;
}
