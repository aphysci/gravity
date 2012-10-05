#include <iostream>
#include "GravityNode.h"
#include "Utility.h"

//Declare a class for receiving Published messages.  
class SimpleGravitySubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
};

int main()
{
	using namespace gravity;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("SimpleGravityComponentID2");

	//Tell the logger to also log to the console.  
	Log::initAndAddConsoleLogger(LogLevel::MESSAGE);
	
	//Subscribe a SimpleGravityHelloWorldSubscriber to the counter.  
	SimpleGravityHelloWorldSubscriber hwSubscriber;
	gn.subscribe("HelloWorldDataProduct", hwSubscriber);
		
	//Wait for us to exit (Ctrl-C or being killed).  
	gn.waitForExit();
	
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
}