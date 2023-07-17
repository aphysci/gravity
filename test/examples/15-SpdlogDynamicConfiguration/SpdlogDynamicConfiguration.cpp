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

class ApplicationLogSubscriber : public GravitySubscriber {
public:
	virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);
};

void logAllAppLevels()
{
	// Wait to ensure it has been sent and received
	sleep(2000);
	spdlog::trace("App");
	spdlog::debug("App");
	spdlog::info("App");
	spdlog::warn("App");
	spdlog::error("App");
	spdlog::critical("App");
}

void sendConfigMessage(GravityNode &gn, std::string compID, GravitySpdLogConfigPB_LoggerType lt, GravitySpdLogConfigPB_LoggerLevel ll)
{
	//Create a data product to send across the network
	GravityDataProduct gravityConfigDP(gravity::constants::SPD_LOG_CONFIG_DPID);
		
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
		spdlog::error("Could not publish data product with id {}", gravity::constants::SPD_LOG_CONFIG_DPID);

	}
}

int main()
{
	GravityNode gn;

	//Initialize gravity, giving this node a componentID.
	if (gn.init("SimpleGravityComponentID") != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not initialize GravityNode");
		exit(1);
	}

	//Register a data product
    if (gn.registerDataProduct(gravity::constants::SPD_LOG_CONFIG_DPID, GravityTransportTypes::TCP) != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not register data product with id {}", gravity::constants::SPD_LOG_CONFIG_DPID);
		exit(1);
	}
	
	// Set up a subscriber to the Application Publisher Logger
	ApplicationLogSubscriber simpleSubscriber;
	gn.subscribe(gravity::constants::GRAVITY_LOGGER_DPID, simpleSubscriber);

	std::cout<<"Begin testing GravityLogger\n";
	
	// Change SimpleGravityComponentID's File to trace level
	// for gravity and trace level for the console
	sendConfigMessage(gn,"", GravitySpdLogConfigPB_LoggerType_GravityFileLogger, GravitySpdLogConfigPB_LoggerLevel_trace);
	sendConfigMessage(gn,"SimpleGravityComponentID", GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_trace);

	logAllAppLevels(); // nothing prints

	// Turns off file and console for GravityLogger
	// for SimpleGravityComponentID and ServiceDirectory
	sendConfigMessage(gn, "SimpleGravityComponentID", GravitySpdLogConfigPB_LoggerType_GravityConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn, "", GravitySpdLogConfigPB_LoggerType_GravityFileLogger, GravitySpdLogConfigPB_LoggerLevel_off);

	std::cout << "End testing GravityLogger\n";

	// Nothing changes as NotARealID does not work
	sendConfigMessage(gn, "NotARealID", GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger, GravitySpdLogConfigPB_LoggerLevel_error);
	logAllAppLevels(); 

	
	std::cout << "Begin testing ApplicationLogger\n";

	// Change SimpleGravityComponentID's network to critical level 
	// for ApplicationLogger and any subscribed components' console to info level
	// and file to warn level
	sendConfigMessage(gn, "SimpleGravityComponentID", GravitySpdLogConfigPB_LoggerType_ApplicationNetworkLogger, GravitySpdLogConfigPB_LoggerLevel_critical);
	sendConfigMessage(gn, "", GravitySpdLogConfigPB_LoggerType_ApplicationConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_info);
	sendConfigMessage(gn, "", GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger, GravitySpdLogConfigPB_LoggerLevel_warn);
	logAllAppLevels();

	// Change all logger's changed back to off
	sendConfigMessage(gn, "SimpleGravityComponentID1", GravitySpdLogConfigPB_LoggerType_ApplicationNetworkLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn, "", GravitySpdLogConfigPB_LoggerType_ApplicationConsoleLogger, GravitySpdLogConfigPB_LoggerLevel_off);
	sendConfigMessage(gn, "", GravitySpdLogConfigPB_LoggerType_ApplicationFileLogger, GravitySpdLogConfigPB_LoggerLevel_off);

	logAllAppLevels();

	std::cout<<"End testing ApplicationLogger\n";

	gn.unsubscribe(gravity::constants::GRAVITY_LOGGER_DPID, simpleSubscriber);
}

void ApplicationLogSubscriber::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts)
{
	for(std::vector< std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
	 				i != dataProducts.end(); i++)
		{
			//Get the protobuf object from the message
			GravityLogMessagePB logMessage;
			(*i)->populateMessage(logMessage);

			std::string message = logMessage.message();
			// Use a substring to cut off the time and level info (metadata) included in log string
			if(message.substr(message.length()-4,3) == "App" && logMessage.level() == "critical")
			{
				std::cout << "The correct level and message were published for Application Network Logger\n";
			}
		}
}