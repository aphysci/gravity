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

using namespace gravity;

class Provider : public GravityServiceProvider
{
public:
    int counter;
	virtual shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
};


shared_ptr<GravityDataProduct> Provider::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
	//Just to be safe.  In theory this can never happen unless this class is registered with more than one serviceID types.
	if(dataProduct.getDataProductID() != "Counter") {
		Log::critical("Request is not for counter!");
		return shared_ptr<GravityDataProduct>(new GravityDataProduct("BadRequest"));
	}

	Log::warning("Request received");

	shared_ptr<GravityDataProduct> resultDP(new GravityDataProduct("Result"));
	resultDP->setData(&counter, sizeof(int));
	counter++;

	return resultDP;
}

int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.
	gn.init("Provider");

	Provider msp;
	msp.counter = 0;
	gn.registerService(
						//This identifies the Service to the service directory so that others can
						// make a request to it.
						"Counter",
						//Assign a transport type to the socket (almost always tcp, unless you are only
						//using the gravity data product between two processes on the same computer).
						GravityTransportTypes::TCP,
						//Give an instance of the service class to be called when a request is made.
						msp);

	//Wait for us to exit (Ctrl-C or being killed).
	gn.waitForExit();

	gn.unregisterService("Counter");
}
