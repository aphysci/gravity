#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

#include "../protobuf/Multiplication.pb.h"

using namespace gravity;


bool gotAsyncMessage = false;

//After multiplication is requested, this class may be called with the result.  
class MultiplicationRequestor : public GravityRequestor
{
public:
	virtual void requestFilled(string serviceID, string requestID, const GravityDataProduct& response);
};

void MultiplicationRequestor::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
	//Parse the message into a protobuf.  
	MultiplicationResultPB result;
	response.populateMessage(result);
	
	//Write the answer
	Log::message("%s: %d", requestID.c_str(), result.result());
	
	gotAsyncMessage = true;
}

int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("MultiplicationRequestor");
	
	// Tell the logger to also log to the console.  
	Log::initAndAddConsoleLogger(Log::MESSAGE);	

	/////////////////////////////
	// Set up the first multiplication request
	MultiplicationRequestor requestor;
	
	GravityDataProduct multRequest1("Multiplication");
	MultiplicationOperandsPB params1;
	params1.set_multiplicand_a(8);
	params1.set_multiplicand_b(2);
	multRequest1.setData(params1);
	
	// Make an Asyncronous request for multiplication
	gn.request("Multiplication", //Service Name
				multRequest1, //Request
				requestor, //Object containing callback that will get the result.  
				"8 x 2"); //A string that identifies which request this is.  

	/////////////////////////////////////////
	//Set up the second multiplication request
	GravityDataProduct multRequest2("Multiplication");
	MultiplicationOperandsPB params2;
	params2.set_multiplicand_a(5);
	params2.set_multiplicand_b(7);
	multRequest2.setData(params2);
	
	//Make a Synchronous request for multiplication
	shared_ptr<GravityDataProduct> multSync = gn.request("Multiplication", //Service Name
														multRequest2, //Request
														1000); //Timeout in milliseconds
	if(multSync == NULL)
	{
		Log::critical("Request Returned NULL!");
	}
	else
	{
		MultiplicationResultPB result;
		multSync->populateMessage(result);
		
		Log::message("5 x 7 = %d", result.result());
	}

	/////////////////////////////////////////
	//Wait for the Asynchronous message to come in.  
	while(!gotAsyncMessage)
	{
		gravity::sleep(1000);
	}
	
	return 0;
}
