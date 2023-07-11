#include <iostream>
#include <GravityNode.h>
#include <GravitySubscriber.h>
#include "protobuf/GravitySpdLogConfigPB.pb.h"
#include "protobuf/GravityLogMessagePB.pb.h"

using namespace std;
using namespace gravity;

#ifdef WIN32
#include <Windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#endif

class SimpleSubscriber : public GravitySubscriber {
public:
	virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);
};

void logAllAppLevels()
{
	spdlog::trace("AppTraceLog");
	spdlog::debug("AppDebugLog");
	spdlog::info("AppInfoLog");
	spdlog::warn("AppWarnLog");
	spdlog::error("AppErrorLog");
	spdlog::critical("AppCriticalLog");
}

void sendConfigMessage(GravityNode &gn,std::string dataProductID,std::string compID, GravitySpdLogConfigPB_LoggerType lt, GravitySpdLogConfigPB_LoggerLevel ll)
{
	//Create a data product to send across the network
	GravityDataProduct gravityConfigDP(dataProductID);
		
    //Initialize our message
	GravitySpdLogConfigPB spdLogConfigPB;
	if(compID != "")
	{
		spdLogConfigPB.set_component_id(compID);
	}
    
	spdLogConfigPB.set_logger_id(lt);
    spdLogConfigPB.set_logger_level(ll);
    gravityConfigDP.setData(spdLogConfigPB);
		
    //Publish the  data product.
	if (gn.publish(gravityConfigDP) != GravityReturnCodes::SUCCESS)
	{
		spdlog::error("Could not publish data product with id {}", dataProductID);
	}
}

int main()
{
    using namespace gravity;

	GravityNode gn1;
	GravityNode gn2;

	//Initialize gravity, giving this node a componentID.
	if (gn1.init("SimpleGravityComponentID1") != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not initialize GravityNode");
		exit(1);
	}
	if (gn2.init("SimpleGravityComponentID2") != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not initialize GravityNode");
		exit(1);
	}

	//Register a data product
	const std::string dataProductID = "GravitySpdLogConfig";
    if (gn1.registerDataProduct(dataProductID, GravityTransportTypes::TCP) != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not register data product with id {}", dataProductID);
		exit(1);
	}

	// Set up a subscriber to the Application Publisher Logger
	SimpleSubscriber simpleSubscriber;
	gn1.subscribe(gravity::constants::GRAVITY_LOGGER_DPID, simpleSubscriber);

	sleep(1000); // All components begin as off, nothing should print
	// Change SimpleGravityComponentID1's GravityConsole to trace and file to debug
	sendConfigMessage(gn1, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_trace);
	sendConfigMessage(gn1, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityFileLogger, GravitySpdLogConfigPB_LoggerLevel_debug);
	logAllAppLevels(); // nothing from application should log 

	sleep(1000); // wait for some gravity messages to log (only ID1 is printing this)

	// Change SimpleGravityComponentID1's ApplicationFile to trace and file to debug
	sendConfigMessage(gn1, dataProductID, "SimpleGravityComponentID2", GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger, GravitySpdLogConfigPB_LoggerLevel_error);
	logAllAppLevels(); // only ID2 application file should be logging

	sleep(1000); // wait for some gravity messages to log (only ID1 is printing this)

	// Change both components' ApplicationNetwork  to info
	sendConfigMessage(gn1, dataProductID, "SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_ApplicationNetworkLogger, GravitySpdLogConfigPB_LoggerLevel_critical);
	sendConfigMessage(gn1, dataProductID, "", GravitySpdLogConfigPB_LoggerType_ApplicationConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_info);

	logAllAppLevels(); // both ApplicationConsole should be logging at info, application network for ID1 logging critical 

	// Check subscriber to make sure they are logged
	
	// Change all logger's changed back to off
	sendConfigMessage(gn1, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn1, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityFileLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn1, dataProductID, "SimpleGravityComponentID2", GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn1, dataProductID, "", GravitySpdLogConfigPB_LoggerType_ApplicationNetworkLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	logAllAppLevels(); // none should log anything
	sleep(1000); // wait to allow for some gravity messages to generate (but none should be logged)
}

void SimpleSubscriber::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts)
{
	for(std::vector< std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
	 				i != dataProducts.end(); i++)
		{
			//Get the protobuf object from the message
			GravityLogMessagePB logMessage;
			(*i)->populateMessage(logMessage);

			if(logMessage.message() == "AppCriticalLog" && logMessage.level() == "critical")
			{
				std::string isValid("The correct level and message were published");
				cout << isValid;
			}
		}
}