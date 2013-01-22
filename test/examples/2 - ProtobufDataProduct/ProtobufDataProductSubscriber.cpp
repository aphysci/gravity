#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <GravitySubscriber.h>
#include <Utility.h>

#include "../protobuf/BasicCounterDataProduct.pb.h"

using namespace gravity;

//Declare class for receiving Published messages.  
class SimpleGravityCounterSubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts);
};

int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("SimpleGravityComponentID2");

	//Declare an object of type SimpleGravityCounterSubscriber (this also initilizes the total count to 0).  
	SimpleGravityCounterSubscriber counterSubscriber;
	//Subscribe a SimpleGravityCounterSubscriber to the counter data product.  
	gn.subscribe("BasicCounterDataProduct", counterSubscriber); 

	//Wait for us to exit (Ctrl-C or being killed).  
	gn.waitForExit();

	//Currently this will never be hit because we will have been killed (unfortunately).  
	//But this shouldn't make a difference because the OS should close the socket and free all resources.  
	gn.unsubscribe("BasicCounterDataProduct", counterSubscriber);	
}

void SimpleGravityCounterSubscriber::subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts)
{
	for(std::vector< shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
			i != dataProducts.end(); i++)
	{
		//Get the protobuf object from the message
		BasicCounterDataProductPB counterDataPB;
		(*i)->populateMessage(counterDataPB);

		//Process the message
		Log::warning("Current Count: %d", counterDataPB.count());
	}
}
