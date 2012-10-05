#include <iostream>
#include "GravityNode.h"
#include "GravityLogger.h"
#include "Utility.h"

class MultiplicationServiceProvider : public GravityServiceProvider
{
public:
	virtual shared_ptr<GravityDataProduct> request(const GravityDataProduct& dataProduct);
};


shared_ptr<GravityDataProduct> MultiplicationServiceProvider::request(const GravityDataProduct& dataProduct)
{
	//Just to be safe.  In theory this can never happen unless this class is registered with more than one serviceID types.  
	if(dataProduct.getDataProductID() == "Multiplication") {
		Log::error("Request is not for multiplication!");
		return shared_ptr<GravityDataProduct>(GravityDataProduct("BadRequest"));
	}

	//Get the parameters for this request.  
	MultiplicationOperandsPB params;
	dataProduct.populateMessage(params);
	
	//Do the calculation
	int result = params.multiplicand_a() * params.multiplicand_b();
	
	//Return the results to the requestor
	MultiplicationResultPB result;
	result.set_result(result);
	
	shared_ptr<GravityDataProduct> resultDP(new GravityDataProduct("MultiplicationResult"));
	resultDP.setData(result);

	return resultDP;
}

int main()
{
	using namespace gravity;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("MultiplicationComponent");

	MultiplicationServiceProvider msp;
	gn.registerService(
						//This identifies the Service to the service directory so that others can 
						// make a request to it.  
						"Multiplication", 
						//This assigns a port on this computer to the data product.  No need to remember 
						//this because the service directory will tell this to other components looking 
						//for this data product.  Simply assign a port number between 1024 and 65535 that 
						//is not in use on this machine.  	
						54534, 
						//Assign a transport type to the socket (almost always tcp, unless you are only 
						//using the gravity data product between two processes on the same computer).  
						"tcp", 
						//Give an instance of the multiplication service class to be called when a request is made for multiplication.  
						msp);

	//Wait for us to exit (Ctrl-C or being killed).  
	gn.waitForExit();
	
	//Currently this will never be hit because we will have been killed (unfortunately).  
	//This tells the service directory that the multiplication service is no longer available.  
	gn.unregisterService("Multiplication");
}