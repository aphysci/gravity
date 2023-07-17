#include <memory>
#include <iostream>
#include <stdio.h>
#include "GravitySpdLogConfigSubscriber.h"
#include "spdlog/spdlog.h"
#include "protobuf/GravitySpdLogConfigPB.pb.h"


namespace gravity {
    SpdLogConfigSubscriber::SpdLogConfigSubscriber(){}

	void SpdLogConfigSubscriber::init(std::string compID)
	{
		componentID = compID;
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