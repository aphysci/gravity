#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

#include "../protobuf/BasicCounterDataProductPB.pb.h"

using namespace gravity;

//Declare a class for receiving Published messages.  
class SimpleGravitySubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
};

//Declare another class for receiving Published messages.  
class SimpleGravityCounterSubscriber : public GravitySubscriber
{
public:
	SimpleGravityCounterSubscriber();
	virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
private:
	int countTotals; //Declare a variable to keep track of the sum of all counts received.  
};

//Define the constructor to SimpleGravityCounterSubscriber (called when a SimpleGravityCounterSubscriber object is created).  
SimpleGravityCounterSubscriber::SimpleGravityCounterSubscriber()
{
	countTotals = 0;
}

//Declare another class for receiving Published messages.  
class SimpleGravityHelloWorldSubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
};


int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("SimpleGravityComponentID2");

	//Tell the logger to also log to the console.  
	Log::initAndAddConsoleLogger(Log::MESSAGE);
	
	//Declare an object of type SimpleGravityCounterSubscriber (this also initilizes the total count to 0).  
	SimpleGravityCounterSubscriber counterSubscriber;
	//Subscribe a SimpleGravityCounterSubscriber to the counter data product.  
	gn.subscribe("BasicCounterDataProduct", counterSubscriber); 

	//Subscribe a SimpleGravityHelloWorldSubscriber to the counter.  
	SimpleGravityHelloWorldSubscriber hwSubscriber;
	gn.subscribe("HelloWorldDataProduct", hwSubscriber);
	
	//Subscribe this guy to both data products.  
	SimpleGravitySubscriber	allSubscriber;
	gn.subscribe("BasicCounterDataProduct", allSubscriber);
	gn.subscribe("HelloWorldDataProduct", allSubscriber);
	
	//Wait for us to exit (Ctrl-C or being killed).  
	gn.waitForExit();
}

void SimpleGravityCounterSubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Get the protobuf object from the message
	BasicCounterDataProductPB counterDataPB;
	dataProduct.populateMessage(counterDataPB);
	
	//Process the message
	countTotals = countTotals + counterDataPB.count();

	Log::message("Subscriber 1: Sum of All Counts Received: %d", countTotals);
}

void SimpleGravityHelloWorldSubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Get a raw message
	int size = dataProduct.getDataSize();
	char* message = new char[size + 1];
	dataProduct.getData(message, size);
	message[size] = 0; //Add NULL terminator
	
	//Output the message
	Log::message("Subscriber 2: Got message: %s", message);
	//Don't forget to free the memory we allocated.  
	delete message;
}

void SimpleGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Process Message
	Log::message("Subscriber 3: Got a %s data product", dataProduct.getDataProductID().c_str());

	//Take different actions based on the message type.  
	if(dataProduct.getDataProductID() == "BasicCounterDataProduct")
	{
		BasicCounterDataProductPB counterDataPB;
		dataProduct.populateMessage(counterDataPB);
		Log::debug("Subscriber 3: Current Count: %d", counterDataPB.count());
	}
}
