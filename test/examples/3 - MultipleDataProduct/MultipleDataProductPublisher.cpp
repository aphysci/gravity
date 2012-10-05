#include <iostream>
#include "GravityNode.h"
#include "GravityLogger.h"
#include "Utility.h"

int main()
{
	using namespace gravity;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("SimpleGravityComponentID");

	gn.registerDataProduct(
							//This identifies the Data Product to the service directory so that others can 
							// subscribe to it.  (See BasicDataProductSubscriber.cpp).  
							"BasicCounterDataProduct", 
							//This assigns a port on this computer to the data product.  No need to remember 
							//this because the service directory will tell this to other components looking 
							//for this data product.  Simply assign a port number between 1024 and 65535 that 
							//is not in use on this machine.  
							54531, 
							//Assign a transport type to the socket (almost always tcp, unless you are only 
							//using the gravity data product between two processes on the same computer).  
							"tcp");

	//Register a second data product (for kicks).  
	gn.registerDataProduct("HelloWorldDataProduct", 54532, "tcp");
	
	bool quit = false; //TODO: set this when you want the program to quit if you need to clean up before exiting.  
	int count = 1;
	while(!quit)
	{	
		//Create a data product to send across the network of type "BasicCounterDataProduct".  
		GravityDataProduct counterDataProduct("BasicCounterDataProduct"); //In order to publish, the DataProductID must match one of the registered types.  

		//Initialize our message
		BasicCounterDataProductPB counterDataPB;
		counterDataPB.set_count(count);
		
		//Put message into data product
		counterDataProduct.populateMessage(counterDataPB);

		//Publish the data product.  
		gn.publish(counterDataProduct);
		
		//Increment count
		count++;
		if(count > 50)
			count = 1;

		//Create a second Data Product without using protobufs.  
		GravityDataProduct helloWorldDataProduct("HelloWorldDataProduct");
		std::string data = "Hello World";
		helloWorldDataProduct.setData(data, data.length());
		
		//Publish the second data product.  
		gn.publish(helloWorldDataProduct);

		//A little logging is always nice.  Use gravity.ini to control which logs get written.  
		Log::message("Published message1 with count %d and message2 with data %s", count, data.c_str());
		
		//Sleep for 1 second.  
		gravity::sleep(1000); 
	}

}
