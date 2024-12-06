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
#include <GravitySubscriber.h>
#include <Utility.h>

#include "../protobuf/BasicCounterDataProduct.pb.h"

using namespace gravity;
using namespace std;

//Declare class for receiving Published messages.
class SimpleGravityCounterSubscriber : public GravitySubscriber
{
public:
    virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
};

int main()
{
    GravityNode gn;
    //Initialize gravity, giving this node a componentID.
    GravityReturnCode grc = gn.init("ReplayTest");

    // It's possible that the GravityNode fails to initialize when using Domains because
    // it hasn't heard from the ServiceDirectory.  Looping as we've done here ensures that
    // the component has connected to the ServiceDirectory before it continues to other tasks.
    while (grc != GravityReturnCodes::SUCCESS)
    {
        spdlog::warn("Unable to connect to ServiceDirectory, will try again in 1 second...");
        gravity::sleep(1000);
        grc = gn.init("ReplayTest");
    }

    //Declare an object of type SimpleGravityCounterSubscriber (this also initilizes the total count to 0).
    SimpleGravityCounterSubscriber counterSubscriber;
    //Subscribe a SimpleGravityCounterSubscriber to the counter data product.
    gn.subscribe("BasicCounterDataProduct", counterSubscriber);

    //Wait for us to exit (Ctrl-C or being killed).
    gn.waitForExit();

    //Currently this will never be hit because we will have been killed (unfortunately).
    //But this shouldn't make a difference because the OS should close the socket and free all resources.
    gn.unsubscribe("BasicCounterDataProduct", counterSubscriber);
}

void SimpleGravityCounterSubscriber::subscriptionFilled(
    const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
{
    for (std::vector<std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
         i != dataProducts.end(); i++)
    {
        //Get the protobuf object from the message
        BasicCounterDataProductPB counterDataPB;
        (*i)->populateMessage(counterDataPB);

        //Process the message
        spdlog::warn("Current Count: {}", counterDataPB.count());
    }
}
