#include <memory>
#include <iostream>
#include <stdio.h>
#include "GravitySpdLogConfigSubscriber.h"
#include "spdlog/spdlog.h"
#include "protobuf/GravitySpdLogConfigPB.pb.h"


namespace gravity {
    SpdLogConfigSubscriber::SpdLogConfigSubscriber(){}
	void SpdLogConfigSubscriber::init(std::string compID,std::string fname)
	{
		componentID = compID;
		filename = fname;
	}
    void SpdLogConfigSubscriber::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts) 
    {
        for(std::vector< std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
	    {
		    //Get the protobuf object from the message
		    GravitySpdLogConfigPB spdLogConfigPB;
		    (*i)->populateMessage(spdLogConfigPB);
            // Reconfigure the loggers based on this
		    reconfigSpdLoggers(spdLogConfigPB);
	    }
    }

/*
	void checkFile(GravitySpdLogConfigPB_LoggerType lt,GravitySpdLogConfigPB_LoggerLevel ll,std::string filename)
	{
		bool fileLogger = (lt == GravitySpdLogConfigPB_LoggerType_GravityFileLogger) 
							|| (lt == GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger);
		bool levelOff = ll == GravitySpdLogConfigPB_LoggerLevel_off;
		if(fileLogger && !levelOff)
		{
			FILE *pfile = fopen(filename.c_str(),"a");
			if (pfile==NULL)
			{
				spdlog::error("Could not open/create file");
			}
			else
			{
				fclose(pfile);
			}
		}
	}
*/

    // Determines if correct ID and reconfigures accordingly
    void SpdLogConfigSubscriber::reconfigSpdLoggers(GravitySpdLogConfigPB spdLogConfigPB)
    {
	    // Check if it is one of the specfied components, return if not
	    if (spdLogConfigPB.has_component_id() && spdLogConfigPB.component_id() != componentID)
		{
			return;
		}

		// Create a file if necessary
		//checkFile(spdLogConfigPB.logger_id(),spdLogConfigPB.logger_level(), filename);

		if (spdLogConfigPB.logger_id() == GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger || spdLogConfigPB.logger_id() == GravitySpdLogConfigPB_LoggerType_GravityFileLogger)
		{
			auto log = spdlog::get("GravityLogger");
			log->sinks()[spdLogConfigPB.logger_id()]-> set_level((spdlog::level::level_enum)(spdLogConfigPB.logger_level()));   

		}
		else
		{
			auto log = spdlog::get("GravityApplicationLogger");
			log->sinks()[spdLogConfigPB.logger_id() - 2]-> set_level((spdlog::level::level_enum)(spdLogConfigPB.logger_level()));   
			
		}    
	}
}