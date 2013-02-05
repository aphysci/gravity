#include <iostream>
#include <sstream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

int main()
{
	using namespace gravity;

	GravityNode gn;
	const std::string dataProductID = "HelloWorldDataProduct";

	//Initialize gravity, giving this node a componentID.  
	GravityReturnCode ret = gn.init("SimpleGravityComponentID");
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::fatal("Could not initialize GravityNode, return code was %d", ret);
		exit(1);
	}

	//Register a data product
	ret = gn.registerDataProduct(
							//This identifies the Data Product to the service directory so that others can 
							// subscribe to it.  (See BasicDataProductSubscriber.cpp).  
							dataProductID,
							//Assign a transport type to the socket (almost always tcp, unless you are only 
							//using the gravity data product between two processes on the same computer).  							
							GravityTransportTypes::TCP);
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::fatal("Could not register data product with id %s, return code was %d", dataProductID.c_str(), ret);
		exit(1);
	}
	
	bool quit = false; //TODO: set this when you want the program to quit if you need to clean up before exiting.  
	int count = 1;
	while(!quit)
	{
		//Create a data product to send across the network of type "HelloWorldDataProduct"
		GravityDataProduct helloWorldDataProduct(dataProductID);
		//This is going to be a raw data product (ie not using protobufs).  
		char data[20];
		sprintf(data, "Hello World #%d", count++);
		helloWorldDataProduct.setData((void*)data, strlen(data));
		
		//Publish the  data product.  
		ret = gn.publish(helloWorldDataProduct);
		if (ret != GravityReturnCodes::SUCCESS)
		{
			Log::critical("Could not publish data product with id %s, return code was %d", dataProductID.c_str(), ret);
		}

		//Sleep for 1 second.  
		gravity::sleep(1000);
	}

}
