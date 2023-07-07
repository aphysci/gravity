#include <iostream>
#include <GravityNode.h>

#ifdef WIN32
#include <Windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#endif

// TO DO: make a test with no id, and one with a different id
// Add in the data and see where to log

int main()
{
    using namespace gravity;

	GravityNode gn;
	const std::string dataProductID = "GravitySpdLogConfig";

	//Initialize gravity, giving this node a componentID.
	GravityReturnCode ret = gn.init("SimpleGravityComponentID");
	if (ret != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not initialize GravityNode, return code was {}", ret);
		exit(1);
	}

	//Register a data product
	ret = gn.registerDataProduct(dataProductID, GravityTransportTypes::TCP);
    if (ret != GravityReturnCodes::SUCCESS)
	{
		spdlog::critical("Could not register data product with id {}, return code was {}", dataProductID, ret);
		exit(1);
	}

		//Create a data product to send across the network of type "HelloWorldDataProduct"
		GravityDataProduct gravityConfigDP(dataProductID);
		
        //Initialize our message
		GravitySpdLogConfigPB spdLogConfigPB;
        spdLogConfigPB.set_component_id("SimpleGravityComponentID");
		spdLogConfigPB.set_logger_id();
        spdLogConfigPB.set_logger_level();
        gravityConfigDP.set_data(spdLogConfigPB);
		
        //Publish the  data product.
		ret = gn.publish(gravityConfigDP);
		if (ret != GravityReturnCodes::SUCCESS)
		{
			spdlog::error("Could not publish data product with id {}, return code was {}", dataProductID, ret);
		}

		//Sleep for 1 second.
		gravity::sleep(1000);

}