#include <memory>

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
	    bool isCompID = false;
	    // Check if it is one of the specified components
	    if (spdLogConfigPB.componentid_size() !=0)
	    {
		    for (auto compID : spdLogConfigPB.componentid())
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
		
		    if (spdLogConfigPB.spdlog_component_size() != 0)
		    {
			    for(SpdLogComponentInfoPB logConfig : spdLogConfigPB.spdlog_component())
			    {
				    // Choose logger and max number 
                    // of loggers (2 for gravity, 3 for app)
				    std::shared_ptr<spdlog::logger> log;
				    int max;
				    if (logConfig.isapplogger())
				    {
					    log = gravityApplicationLogger;
					    max = 3;
				    }
				    else
				    {
					    log = gravityLogger;
                        max = 2;
 				    }

				    // Choose sink and set level
				    for (int i = 0 ; i < max; i++)
				    {
					    if(loggerID[i] == logConfig.loggerid())
					    {
						    auto sink = log->sinks()[i];
							sink -> set_level(spdlog::level::from_str(logConfig.log_level()));
						    break;
					    }
				    }
			    }
		    }
	    }
    }
}