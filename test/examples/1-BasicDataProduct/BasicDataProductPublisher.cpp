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
#include <sstream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>
#include "spdlog/spdlog.h"

int main()
{
    using namespace gravity;

    GravityNode gn;
    const std::string dataProductID = "HelloWorldDataProduct";

    //Initialize gravity, giving this node a componentID.
    GravityReturnCode ret = gn.init("SimpleGravityComponentID");
    if (ret != GravityReturnCodes::SUCCESS)
    {
        spdlog::critical("Could not initialize GravityNode, return code was {}", static_cast<int>(ret));
        exit(1);
    }

    //Register a data product
    ret = gn.registerDataProduct(
        //This identifies the Data Product to the service directory so that others can
        // subscribe to it.  (See BasicDataProductSubscriber.cpp).
        dataProductID,
        //Assign a transport type to the socket (almost always tcp, unless you are only
        //using the gravity data product between two processes on the same computer).
        GravityTransportTypes::TCP);
    if (ret != GravityReturnCodes::SUCCESS)
    {
        spdlog::critical("Could not register data product with id {}, return code was {}", dataProductID, static_cast<int>(ret));
        exit(1);
    }

    bool quit = false;  //TODO: set this when you want the program to quit if you need to clean up before exiting.
    int count = 1;
    while (!quit)
    {
        //Create a data product to send across the network of type "HelloWorldDataProduct"
        GravityDataProduct helloWorldDataProduct(dataProductID);
        //This is going to be a raw data product (ie not using protobufs).
        char data[24];
        sprintf(data, "Hello World #%d", count++);
        helloWorldDataProduct.setData((void*)data, strlen(data));

        //Publish the  data product.
        ret = gn.publish(helloWorldDataProduct);
        if (ret != GravityReturnCodes::SUCCESS)
        {
            spdlog::error("Could not publish data product with id {}, return code was {}", dataProductID, static_cast<int>(ret));
        }

        //Sleep for 1 second.
        gravity::sleep(1000);
    }
}
