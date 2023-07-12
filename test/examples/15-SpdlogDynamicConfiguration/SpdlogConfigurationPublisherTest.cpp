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
	spdlog::trace("App");
	spdlog::debug("App");
	spdlog::info("App");
	spdlog::warn("App");
	spdlog::error("App");
	spdlog::critical("App");
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
	std::cout<<"Set logger id " << lt << " and level " << ll << " for " << compID << " and published data product " << dataProductID << "\n";
}

int main()
{
	GravityNode gn;

	//Initialize gravity, giving this node a componentID.
	if (gn.init("SimpleGravityComponentID1") != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not initialize GravityNode");
		exit(1);
	}

	//Register a data product
	const std::string dataProductID = "GravitySpdLogConfig";
    if (gn.registerDataProduct(dataProductID, GravityTransportTypes::TCP) != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not register data product with id {}", dataProductID);
		exit(1);
	}
	std::cout << "Registered data product " << dataProductID << "\n";

	// Set up a subscriber to the Application Publisher Logger
	SimpleSubscriber simpleSubscriber;
	gn.subscribe(gravity::constants::GRAVITY_LOGGER_DPID, simpleSubscriber);

	// Change SimpleGravityComponentID1's GravityConsole to trace and file to debug
	sendConfigMessage(gn, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_trace);
	sendConfigMessage(gn, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityFileLogger, GravitySpdLogConfigPB_LoggerLevel_debug);
	logAllAppLevels(); // nothing from application should log 
	sleep(5000);

	// Nothing changes as component ID does not work
	sendConfigMessage(gn, dataProductID, "NotARealID", GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger, GravitySpdLogConfigPB_LoggerLevel_error);
	logAllAppLevels(); 
	sleep(5000);

	// Change ID1 ApplicationNetwork to critical, any subscribed component ApplicationConsole to info
	sendConfigMessage(gn, dataProductID, "SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_ApplicationNetworkLogger, GravitySpdLogConfigPB_LoggerLevel_critical);
	sendConfigMessage(gn, dataProductID, "", GravitySpdLogConfigPB_LoggerType_ApplicationConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_info);
	logAllAppLevels();
	sleep(5000);

	// Change all logger's changed back to off
	sendConfigMessage(gn, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn, dataProductID,"SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_GravityFileLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn, dataProductID, "SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_ApplicationNetworkLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn, dataProductID, "", GravitySpdLogConfigPB_LoggerType_ApplicationConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	logAllAppLevels();
	sleep(5000);

	gn.waitForExit();
	gn.unsubscribe(gravity::constants::GRAVITY_LOGGER_DPID, simpleSubscriber);
}

void SimpleSubscriber::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts)
{
	for(std::vector< std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
	 				i != dataProducts.end(); i++)
		{
			//Get the protobuf object from the message
			GravityLogMessagePB logMessage;
			(*i)->populateMessage(logMessage);

			std::cout << "Received Application network log message";
			if(logMessage.message() == "App" && logMessage.level() == "critical")
			{
				std::string isValid("The correct level and message were published");
				std::cout << isValid;
			}
		}
}