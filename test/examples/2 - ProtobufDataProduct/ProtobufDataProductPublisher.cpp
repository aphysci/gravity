#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

#include "../protobuf/BasicCounterDataProduct.pb.h"

int main()
{
	using namespace gravity;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("ProtobufGravityComponentID");

	gn.registerDataProduct(
							//This identifies the Data Product to the service directory so that others can 
							// subscribe to it.  (See BasicDataProductSubscriber.cpp).  
							"BasicCounterDataProduct", 
							//Assign a transport type to the socket (almost always tcp, unless you are only 
							//using the gravity data product between two processes on the same computer).  
							"tcp");
	
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
		counterDataProduct.setData(counterDataPB);

		//Publish the data product.  
		gn.publish(counterDataProduct);
		
		//Increment count
		count++;
		if(count > 50)
			count = 1;
		
		//Sleep for 1 second.  
		gravity::sleep(1000); 
	}
}
