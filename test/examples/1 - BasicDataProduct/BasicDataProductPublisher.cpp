#include <iostream>
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
							//This assigns a port on this computer to the data product.  No need to remember 
							//this because the service directory will tell this to other components looking 
							//for this data product.  Simply assign a port number between 1024 and 65535 that 
							//is not in use on this machine.  
							54532, 
							//Assign a transport type to the socket (almost always tcp, unless you are only 
							//using the gravity data product between two processes on the same computer).  							
							"tcp");
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
		std::string data = "Hello World";
		helloWorldDataProduct.setData((void*)data.c_str(), data.length());
		
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
