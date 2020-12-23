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
#include <memory>
#include "GravityTest.h"

using namespace gravity;

//Declare a class for receiving Published messages.
class SimpleGravitySubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);
	bool done;
};

int main()
{
	GravityNode gn;
	const std::string dataProductID = "DataProduct";

	//Initialize gravity, giving this node a componentID.
	GravityReturnCode ret = gn.init("Subscriber");
	int numTries = 3;
	while (ret != GravityReturnCodes::SUCCESS && numTries-- > 0)
	{
	    Log::warning("Error during init, retrying...");
	    ret = gn.init("Subscriber");
	}
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::fatal("Could not initialize GravityNode, return code is %d", ret);
		exit(1);
	}

	//Subscribe to the counter.
	SimpleGravitySubscriber hwSubscriber;
    hwSubscriber.done = false;
	ret = gn.subscribe(dataProductID, hwSubscriber);
	if (ret != GravityReturnCodes::SUCCESS)
	{
		Log::critical("Could not subscribe to data product with id %s, return code was %d", dataProductID.c_str(), ret);
		exit(1);
	}

    int tries = 0;
	while(!hwSubscriber.done && tries++ < 30)
	{
	    Log::trace("done = %d, tries = %d", hwSubscriber.done, tries);
	    gravity::sleep(1000);
	}

	// make sure we finished
	GRAVITY_TEST(tries < 30);

	gn.unsubscribe(dataProductID, hwSubscriber);
}

void SimpleGravitySubscriber::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts)
{
	for(std::vector< std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
			i != dataProducts.end(); i++)
	{
		//Get a raw message
		int counter;
		(*i)->getData(&counter, sizeof(int));

		//Output the message
		Log::warning("Got message: %d", counter);

		if (counter > 20)
		    done = true;
	}
}
