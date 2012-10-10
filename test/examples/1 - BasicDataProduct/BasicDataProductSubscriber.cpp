#include <iostream>
#include "GravityNode.h"
#include "GravityLogger.h"
#include "GravitySubscriber.h"
#include "Utility.h"

using namespace gravity;

//Declare a class for receiving Published messages.  
class SimpleGravityHelloWorldSubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
};

int main()
{
	GravityNode gn;
	const std::string dataProductID = "HelloWorldDataProduct";

	//Tell the logger to also log to the console.
	Log::initAndAddConsoleLogger(Log::MESSAGE);

	//Initialize gravity, giving this node a componentID.  
	GravityReturnCode ret = gn.init("SimpleGravityComponentSubscriber");
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::critical("Could not initialize GravityNode, return code was %d", ret);
		exit(1);
	}

	//Subscribe a SimpleGravityHelloWorldSubscriber to the counter.  
	SimpleGravityHelloWorldSubscriber hwSubscriber;
	ret = gn.subscribe(dataProductID, hwSubscriber);
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::critical("Could not subscribe, return code was %d", ret);
		exit(1);
	}
		
	cout << "Tell publisher to publish... " << endl;
	cin.ignore();
	
	//Currently this will never be hit because we will have been killed (unfortunately).  
	//But this shouldn't make a difference because the OS should close the socket and free all resources.  
	gn.unsubscribe("HelloWorldDataProduct", hwSubscriber);
}

void SimpleGravityHelloWorldSubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Get a raw message
	int size = dataProduct.getDataSize();
	char* message = new char[size];
	dataProduct.getData(message, size);
	
	//Output the message
	Log::message("Got message: %s", message);
	//Don't forget to free the memory we allocated.  
	delete message;

	cout << "Press enter to exit..." << endl;
}
