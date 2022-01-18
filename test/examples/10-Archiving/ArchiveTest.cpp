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

#include "../protobuf/BasicCounterDataProduct.pb.h"
#include "protobuf/FileArchiverControlRequestPB.pb.h"

int main()
{
	using namespace gravity;
    using namespace std;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.
	GravityReturnCode grc = gn.init("ArchiveTest");

	// It's possible that the GravityNode fails to initialize when using Domains because
	// it hasn't heard from the ServiceDirectory.  Looping as we've done here ensures that
	// the component has connected to the ServiceDirectory before it continues to other tasks.
	while (grc != GravityReturnCodes::SUCCESS)
	{
	    spdlog::warn("Unable to connect to ServiceDirectory, will try again in 1 second...");
	    gravity::sleep(1000);
	    grc = gn.init("ArchiveTest");
	}

	gn.registerDataProduct(
                            //This identifies the Data Product to the service directory so that others can
							// subscribe to it.  (See BasicDataProductSubscriber.cpp).
							"BasicCounterDataProduct",
							//Assign a transport type to the socket (almost always tcp, unless you are only
							//using the gravity data product between two processes on the same computer).
							GravityTransportTypes::TCP);

	bool quit = false; //TODO: set this when you want the program to quit if you need to clean up before exiting.
	int count = 1;
	bool suspend = true;
	while(!quit)
	{
		//Create a data product to send across the network of type "BasicCounterDataProduct".
		GravityDataProduct counterDataProduct("BasicCounterDataProduct"); //In order to publish, the DataProductID must match one of the registered types.

		//Initialize our message
		BasicCounterDataProductPB counterDataPB;
		counterDataPB.set_count(count);

		//Put message into data product
		counterDataProduct.setData(counterDataPB);

		//Publish the data product.
		gn.publish(counterDataProduct);
		spdlog::info("published count = {}", count);

		//Increment count
		count++;
		if(count > 50)
			count = 1;

		if (count % 5 == 0)
		{
		    GravityDataProduct gdp("FileArchiverControlRequest");
		    FileArchiverControlRequestPB request;
		    request.set_suspend(suspend);
		    suspend = !suspend;
		    gdp.setData(request);
		    gn.request("FileArchiverControlRequest", gdp);
		}

		//Sleep for 1 second.
		gravity::sleep(1000);
	}
}
