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

class MultiplicationServiceProvider : public GravityServiceProvider
{
public:
	virtual tr1::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
};


tr1::shared_ptr<GravityDataProduct> MultiplicationServiceProvider::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
	//Just to be safe.  In theory this can never happen unless this class is registered with more than one serviceID types.
	if(dataProduct.getDataProductID() != "Multiplication") {
		Log::critical("Request is not for multiplication!");
		return tr1::shared_ptr<GravityDataProduct>(new GravityDataProduct("BadRequest"));
	}

	//Get the parameters for this request.
	MultiplicationOperandsPB params;
	dataProduct.populateMessage(params);

	Log::warning("Request received: %d x %d", params.multiplicand_a(), params.multiplicand_b());

	//Do the calculation
	int result = params.multiplicand_a() * params.multiplicand_b();

	//Return the results to the requestor
	MultiplicationResultPB resultPB;
	resultPB.set_result(result);

	tr1::shared_ptr<GravityDataProduct> resultDP(new GravityDataProduct("MultiplicationResult"));
	resultDP->setData(resultPB);

	return resultDP;
}

int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.
	gn.init("MultiplicationComponent");

	MultiplicationServiceProvider msp;
	gn.registerService(
						//This identifies the Service to the service directory so that others can
						// make a request to it.
						"Multiplication",
						//Assign a transport type to the socket (almost always tcp, unless you are only
						//using the gravity data product between two processes on the same computer).
						GravityTransportTypes::TCP,
						//Give an instance of the multiplication service class to be called when a request is made for multiplication.
						msp);

	//Wait for us to exit (Ctrl-C or being killed).
	gn.waitForExit();

	//Currently this will never be hit because we will have been killed (unfortunately).
	//This tells the service directory that the multiplication service is no longer available.
	gn.unregisterService("Multiplication");
}
