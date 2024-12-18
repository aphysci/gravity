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

#include "GravityLogger.h"
#include "FileReader.h"
#include "FileReplay.h"
#include "SpdLog.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <thread>
#ifndef WIN32
#include <unistd.h>
#endif

using namespace std;

int main(int argc, const char* argv[])
{
    gravity::FileReplay replay;
    replay.waitForExit();
}

namespace gravity
{

const char* FileReplay::ComponentName = "FileReplay";

FileReplay::FileReplay()
{
    // Initialize Gravity Node
    gravityNode.init(FileReplay::ComponentName);

    // Get Gravity logger
    logger = gravityNode.getGravityLogger();
    if (!logger)
    {
        SpdLog::critical("Failed to get GravityLogger");
        return;
    }

    // Configure logger to log to console
    //Log::initAndAddConsoleLogger(FileReplay::ComponentName, Log::MESSAGE);

    // Initialize firstPublishTime
    firstPublishTime = 0;
    firstDataTime = 0;

    // Get archive filename
    string filename = gravityNode.getStringParam("ArchiveFilename", "archive.bin");

    string dpList = gravityNode.getStringParam("DataProductList", "");

    // Kick off the thread to start reading the file
    fileReader.init(filename, dpList);
    std::thread readerThread(&FileReader::start, &fileReader);
    readerThread.detach();

    // Start processing the archive file
    processArchive();
}

FileReplay::~FileReplay() {}

void FileReplay::processArchive()
{
    std::shared_ptr<GravityDataProduct> gdp = fileReader.getNextDataProduct();
    while (gdp)
    {
        // Ensure that we've registered this data product
        set<string>::iterator iter = datatypes.find(gdp->getDataProductID());
        if (iter == datatypes.end())
        {
            gravityNode.registerDataProduct(gdp->getDataProductID(), GravityTransportTypes::TCP);
            datatypes.insert(gdp->getDataProductID());
        }

        // Wait the appropriate amount of time and then publish this data product
        uint64_t timestamp = gdp->getReceivedTimestamp();
        if (timestamp == 0)
        {
            timestamp = gdp->getGravityTimestamp();
        }
        if (firstPublishTime == 0)
        {
            // This is the first one
            firstPublishTime = gravity::getCurrentTime();
            firstDataTime = timestamp;
        }
        else
        {
            uint64_t timeToWait = timestamp > firstDataTime ? timestamp - firstDataTime : 0;
            uint64_t elapsedTime = gravity::getCurrentTime() - firstPublishTime;
            if (elapsedTime < timeToWait)
            {
                logger->debug("waiting {}", timeToWait - elapsedTime);
#ifdef WIN32
                gravity::sleep((timeToWait - elapsedTime) / 1000);
#else
                usleep(timeToWait - elapsedTime);
#endif
            }
        }

        // publish
        logger->debug("publishing {}", gdp->getDataProductID());
        gravityNode.publish(*gdp);

        // Get the next one
        gdp = fileReader.getNextDataProduct();
    }

    logger->info("Reached end of archive");
}

void FileReplay::waitForExit() { gravityNode.waitForExit(); }

} /* namespace gravity */
