#include <iostream>
#include "GravityNode.h"
#include "GravityLogger.h"
#include "Utility.h"

int main()
{
	using namespace gravity;

	const std::string dataProductID = "HelloWorldDataProduct";

	cout << "in publish " << endl;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	GravityReturnCode ret = gn.init("SimpleGravityComponentPublisher");
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::critical("Could not initialize GravityNode, return code was %s", ret);
		exit(1);
	}

	//Register a data product
	gn.registerDataProduct(
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
	
	//Create a data product to send across the network of type "HelloWorldDataProduct"
	GravityDataProduct helloWorldDataProduct(dataProductID);
	//This is going to be a raw data product (i.e. not using protobufs).
	std::string data = "Hello World";

	cout << "press key after subscriber is started..." << endl;
	cin.ignore();

	// convert the std::string to a char*
	helloWorldDataProduct.setData(data.c_str(), data.length());

	//Publish the  data product.
	ret = gn.publish(helloWorldDataProduct);
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::critical("Could not publish, return code was %s", ret);
		exit(1);
	}

	cout << "press key to exit..." << endl;
	cin.ignore();

}
