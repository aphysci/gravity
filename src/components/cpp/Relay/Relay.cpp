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
#include <SpdLog.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include "spdlog/spdlog.h"

gravity::Semaphore relayLock;
bool quit = false;

#ifdef WIN32
#include <windows.h>
BOOL WINAPI consoleHandler(DWORD sig)
{
    if (sig == CTRL_C_EVENT)
    {
        relayLock.Lock();
        quit = true;
        relayLock.Unlock();
    }

    return TRUE;
}

#else
#include <signal.h>
void handler(int sig)
{
    relayLock.Lock();
    quit = true;
    relayLock.Unlock();
}
#endif

#define COMPONENT_ID "Relay"

using namespace gravity;
using namespace std;

class Relay : public GravitySubscriber
{
private:
    GravityNode gravityNode;
    shared_ptr<spdlog::logger> logger;

public:
    Relay(){};
    virtual ~Relay(){};

    int run();
    void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
};

int Relay::run()
{
    GravityReturnCode ret = gravityNode.init(COMPONENT_ID);
    while (ret != GravityReturnCodes::SUCCESS)
    {
        cerr << "Failed to initialize " << COMPONENT_ID << ", retrying..." << endl;
        ret = gravityNode.init(COMPONENT_ID);
    }

    // Get Gravity logger
    logger = gravityNode.getGravityLogger();
    if (!logger)
    {
        SpdLog::critical("Failed to get GravityLogger");
        return 1;
    }

    bool localOnly = gravityNode.getBoolParam("ProvideLocalOnly", true);

    // Get list of data products to archive
    string dpList = gravityNode.getStringParam("DataProductList", "");

    // Subscribe to each data product
    std::istringstream tokenStream(dpList);
    string token;
    list<string> dataProducts;
    while (getline(tokenStream, token, ','))
    {
        // trim any spaces from the ends
        token.erase(token.find_last_not_of(" ") + 1);
        token.erase(0, token.find_first_not_of(" "));

        logger->debug("Configured to relay: {}", token);
        gravityNode.registerRelay(token, *this, localOnly, GravityTransportTypes::TCP);
        dataProducts.push_back(token);
    }

#ifdef WIN32
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE))
    {
        logger->error("ERROR: Could not set control handler - Relay will not be able to unregister when terminated");
    }
#else
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);
#endif

    while (true)
    {
        gravity::sleep(1000);
        relayLock.Lock();
        if (quit) break;
        relayLock.Unlock();
    }

    logger->warn("Exiting, but cleaning up registrations first...");
    for (list<string>::iterator iter = dataProducts.begin(); iter != dataProducts.end(); iter++)
    {
        gravityNode.unregisterRelay(*iter, *this);
    }
    logger->warn("...done");

    return 0;
}

/**
 * Not much happens here, it just passes along the GDP's that it receives.  The main work is done in the ServiceDirectory
 * where it recognizes subscribers that are collocated with the Relay and just provides data from there.
 */
void Relay::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
{
    for (unsigned int i = 0; i < dataProducts.size(); i++)
    {
        std::shared_ptr<GravityDataProduct> dataProduct = dataProducts.at(i);
        logger->debug("Republishing {}", dataProduct->getDataProductID());
        dataProduct->setIsRelayedDataproduct(true);
        gravityNode.publish(*dataProduct);
    }
}

int main(int argc, const char** argv)
{
    Relay relay;

    // Need to explicitly call exit here since we're capturing SIGINT and SIGTERM.  Otherwise, if the SD
    // goes away while we're trying to unregister, this could hang indefinitely.
    exit(relay.run());
}
