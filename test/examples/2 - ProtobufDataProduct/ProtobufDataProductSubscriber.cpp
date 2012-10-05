#include <iostream>
#include "GravityNode.h"
#include "Utility.h"

//Declare class for receiving Published messages.  
class SimpleGravityCounterSubscriber : public GravitySubscriber
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
	
	//Declare an object of type SimpleGravityCounterSubscriber (this also initilizes the total count to 0).  
	SimpleGravityCounterSubscriber counterSubscriber();
	//Subscribe a SimpleGravityCounterSubscriber to the counter data product.  
	gn.subscribe("BasicCounterDataProduct", counterSubscriber); 

	//Wait for us to exit (Ctrl-C or being killed).  
	gn.waitForExit();

	//Currently this will never be hit because we will have been killed (unfortunately).  
	//But this shouldn't make a difference because the OS should close the socket and free all resources.  
	gn.unsubscribe("BasicCounterDataProduct", counterSubscriber);	
}

void SimpleGravityCounterSubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Get the protobuf object from the message
	BasicCounterDataProductPB counterDataPB;
	dataProduct.populateMessage(counterDataPB);
	
	//Process the message
	Log::message("Current Count: %d", counterDataPB.count());
}