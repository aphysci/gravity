/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

#include "../protobuf/Multiplication.pb.h"

using namespace gravity;
using namespace std;

bool gotAsyncMessage = false;

//After multiplication is requested, this class may be called with the result.
class MultiplicationRequestor : public GravityRequestor
{
public:
	virtual void requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response);
};

void MultiplicationRequestor::requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response)
{
	//Parse the message into a protobuf.
	MultiplicationResultPB result;
	response.populateMessage(result);

	//Write the answer
	Log::warning("Asynchronous response received: %s = %d", requestID.c_str(), result.result());

	gotAsyncMessage = true;
}

int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.
	GravityReturnCode ret = gn.init("MultiplicationRequestor");
	while (ret != GravityReturnCodes::SUCCESS)
	{
	    Log::warning("Unable to init component, retrying...");
	    ret = gn.init("MultiplicationRequestor");
	}

	/////////////////////////////
	// Set up the first multiplication request
	MultiplicationRequestor requestor;

	GravityDataProduct multRequest1("Multiplication");
	MultiplicationOperandsPB params1;
	params1.set_multiplicand_a(8);
	params1.set_multiplicand_b(2);
	multRequest1.setData(params1);

	// Make an Asynchronous request for multiplication
	do
	{
        ret = gn.request("Multiplication", //Service Name
                         multRequest1, //Request
                         requestor, //Object containing callback that will get the result.
                         "8 x 2"); //A string that identifies which request this is.
        // Service may not be registered yet
        if (ret != GravityReturnCodes::SUCCESS)
        {
            Log::warning("request to Multiplication service failed, retrying...");
            gravity::sleep(1000);
        }
	}
    while (ret != GravityReturnCodes::SUCCESS);

	/////////////////////////////////////////
	//Set up the second multiplication request
	GravityDataProduct multRequest2("Multiplication");
	MultiplicationOperandsPB params2;
	params2.set_multiplicand_a(5);
	params2.set_multiplicand_b(7);
	multRequest2.setData(params2);

	//Make a Synchronous request for multiplication
	std::shared_ptr<GravityDataProduct> multSync = gn.request("Multiplication", //Service Name
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

		Log::warning("Synchronous response received: 5 x 7 = %d", result.result());
	}

	/////////////////////////////////////////
	//Wait for the Asynchronous message to come in.
	while(!gotAsyncMessage)
	{
		gravity::sleep(1000);
	}

	return 0;
}
