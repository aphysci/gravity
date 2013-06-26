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
    Log::message("ConfigFileLogLevel = %s\n", gn.getStringParam( "ConfigFileLogLevel", "Not Found" ).c_str() );
    Log::message("bin_ms = %f\n", gn.getFloatParam( "bin_ms", 0. ) );
    Log::message("bin_us = %f\n", gn.getFloatParam( "bin_us", 0. ) );
    Log::message("win_ms = %d\n", gn.getIntParam( "win_ms", 0 ) );
    Log::message("na_samples = %d\n", gn.getIntParam( "na_samples", 0 ) );
    Log::message("nsamps = %d\n", gn.getIntParam( "nsamps", 0 ) );
    Log::message("nsamps_minus = %d\n", gn.getIntParam( "nsamps_minus", 0 ) );
    Log::message("operatorstr = %s\n", gn.getStringParam( "operatorstr", "Not found" ).c_str() );
    return 0;
}
