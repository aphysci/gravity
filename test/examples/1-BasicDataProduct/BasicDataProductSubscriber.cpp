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
#include "spdlog/spdlog.h"

using namespace gravity;

//Declare a class for receiving Published messages.
class SimpleGravitySubscriber : public GravitySubscriber
{
public:
    virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
};

int main()
{
    GravityNode gn;
    const std::string dataProductID = "HelloWorldDataProduct";

    //Initialize gravity, giving this node a componentID.
    GravityReturnCode ret = gn.init("SimpleGravityComponentID2");
    if (ret != GravityReturnCodes::SUCCESS)
    {
        spdlog::critical("Could not initialize GravityNode, return code is {}", ret);
        exit(1);
    }

    //Subscribe a SimpleGravityHelloWorldSubscriber to the counter.
    SimpleGravitySubscriber hwSubscriber;
    ret = gn.subscribe(dataProductID, hwSubscriber);
    if (ret != GravityReturnCodes::SUCCESS)
    {
        spdlog::error("Could not subscribe to data product with id {}, return code was {}", dataProductID, ret);
        exit(1);
    }

    //Wait for us to exit (Ctrl-C or being killed).
    gn.waitForExit();

    //Currently this will never be hit because we will have been killed (unfortunately).
    //But this shouldn't make a difference because the OS should close the socket and free all resources.
    gn.unsubscribe("HelloWorldDataProduct", hwSubscriber);
}

void SimpleGravitySubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
{
    for (std::vector<std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
         i != dataProducts.end(); i++)
    {
        //Get a raw message
        int size = (*i)->getDataSize();
        char* message = new char[size + 1];
        (*i)->getData(message, size);
        message[size] = 0;  // null terminate

        //Output the message
        spdlog::warn("Got message: {}", message);
        //Don't forget to free the memory we allocated.
        delete[] message;
    }
}
