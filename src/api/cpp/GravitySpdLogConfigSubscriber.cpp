#include "GravitySpdLogConfigSubscriber.h"
#include "spdlog/spdlog.h"
#include "protobuf/GravitySpdLogConfigPB.pb.h"


namespace gravity {
    SpdLogConfigSubscriber::SpdLogConfigSubscriber(){}
	spdLogConfigSubscriber::init(std::string compID)
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
	    bool isCompID = false;
	    // Check if it is one of the specified components
	    if (spdLogConfigPB.has_componentID())
	    {
		    for (auto compID : spdLogConfigPB.componentID())
		    {
			    if (compID == componentID)
			    {
				    isCompID = true;
				    break;
			    }
		    }   
	    }
	    // All components subscribed should be updated
	    else
        {
		    isCompID = true;
	    }

        // Update valid component
	    if (isCompID){
		    // Get the gravity and application loggers
		    auto gravityLogger = spdlog::get("GravityLogger");
		    auto gravityApplicationLogger = spdlog::get("GravityApplicationLogger");

		    std::string loggerID[3] = {"console","file", "network"};
		
		    if (spdLogConfigPB.has_spdlog_component());
		    {
			    for(SpdLogComponentInfoPB logConfig : spdLogConfigPB.spdlog_component());
			    {
				    // Choose logger and max number 
                    // of loggers (2 for gravity, 3 for app)
				    auto logger;
				    int max;
				    if (logConfig.isAppLogger())
				    {
					    logger = gravityApplicationLogger;
					    max = 3;
				    }
				    else
				    {
					    logger = gravityLogger;
                        max = 2;
 				    }

				    // Choose sink and set level
				    for (int i = 0 ; i < max; i++)
				    {
					    if(loggerID[i] == logConfig.loggerID())
					    {
						    logger->sink()[i] -> set_level(logConfig.log_level());
						    break;
					    }
				    }
			    }
		    }
	    }
    }
}