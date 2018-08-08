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
#include "GravityTest.h"

using namespace gravity;

int main()
{
	GravityNode gn;
	//Initialize gravity, giving this node a componentID.
	GravityReturnCode ret = gn.init("Requestor");
    int numTries = 3;
    while (ret != GravityReturnCodes::SUCCESS && numTries-- > 0)
    {
        Log::warning("Error during init, retrying...");
        ret = gn.init("Requestor");
    }
    if (ret != GravityReturnCodes::SUCCESS)
    {
        Log::fatal("Could not initialize GravityNode, return code was %d", ret);
        exit(1);
    }


	GravityDataProduct request("Counter");

    int counter = 0;
    int tries = 0;
	while(counter < 20 && tries++ < 30)
	{
	    std::tr1::shared_ptr<GravityDataProduct> countReq = gn.request("Counter", //Service Name
                                                             request, //Request
                                                             3000); //Timeout in milliseconds
        if(countReq == NULL)
        {
            Log::critical("Request Returned NULL!");
        }
        else
        {
            countReq->getData(&counter, sizeof(int));
            Log::warning("Synchronous response received: %d", counter);
        }

        gravity::sleep(1000);
	}

    // make sure we finished
    GRAVITY_TEST(tries < 30);

	return 0;
}
