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

#include <GravityNode.h>
#include <GravityLogger.h>
#include <protobuf/RegisterRelay.pb.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

using namespace gravity;
using namespace std;
using namespace std::tr1;

#define COMPONENT_ID "Relay"

class Relay : public GravitySubscriber
{
private:
    GravityNode gravityNode;

public:
    Relay() {};
    virtual ~Relay() {};

    int run();
    void subscriptionFilled(const std::vector< std::tr1::shared_ptr<GravityDataProduct> >& dataProducts);
};

int Relay::run()
{
    GravityReturnCode ret = gravityNode.init(COMPONENT_ID);
    while (ret != GravityReturnCodes::SUCCESS)
    {
        cerr << "Failed to initialize " << COMPONENT_ID << ", retrying..." << endl;
        ret = gravityNode.init(COMPONENT_ID);
    }

    RegisterRelayPB registerRelay;
    registerRelay.set_componentid(COMPONENT_ID);

    if (gravityNode.getBoolParam("ProvideLocalOnly", true))
    {
    	registerRelay.set_ipaddress(gravityNode.getIP());
    }

    // Get list of data products to archive
    string dpList = gravityNode.getStringParam("DataProductList", "");

    // Subscribe to each data product
    std::istringstream tokenStream(dpList);
    string token;
    while (getline(tokenStream, token, ','))
    {
        // trim any spaces from the ends
        token.erase(token.find_last_not_of(" ") + 1);
        token.erase(0, token.find_first_not_of(" "));

        Log::debug("Configured to relay: %s", token.c_str());
        registerRelay.add_dataproductids(token);
    }

    GravityDataProduct gdp("RegisterRelay");
    gdp.setData(registerRelay);
    shared_ptr<GravityDataProduct> retGDP = gravityNode.request("RegisterRelay", gdp, 2000);
    while (retGDP == NULL)
    {
    	Log::warning("Service request to register relay failed, retrying...");
    	retGDP = gravityNode.request("RegisterRelay", gdp, 2000);
    }

    for (int i = 0; i < registerRelay.dataproductids_size(); i++)
    {
        gravityNode.registerDataProduct(registerRelay.dataproductids(i), GravityTransportTypes::TCP);
        gravityNode.subscribe(registerRelay.dataproductids(i), *this);
    }

    gravityNode.waitForExit();

    return 0;
}

/**
 * Not much happens here, it just passes along the GDP's that it receives.  The main work is done in the ServiceDirectory
 * where it recognizes subscribers that are collocated with the Relay and just provides data from there.
 */
void Relay::subscriptionFilled(const std::vector< std::tr1::shared_ptr<GravityDataProduct> >& dataProducts)
{
    for (unsigned int i = 0; i < dataProducts.size(); i++)
    {
        shared_ptr<GravityDataProduct> dataProduct = dataProducts.at(i);
        Log::debug("Republishing %s", dataProduct->getDataProductID().c_str());
        dataProduct->setIsRelayedDataproduct(true);
        gravityNode.publish(*dataProduct);
    }
}

int main(int argc, const char** argv)
{
	Relay relay;
	return relay.run();
}
