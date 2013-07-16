/*
 * FileReplay.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: esmf
 */

#include "GravityLogger.h"
#include "FileReader.h"
#include "FileReplay.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>

int main(int argc, const char* argv[])
{
	esmf::FileReplay replay;
	replay.waitForExit();
}

namespace esmf {

const char* FileReplay::ComponentName = "FileReplay";

FileReplay::FileReplay()
{    
	// Initialize Gravity Node
	gravityNode.init(FileReplay::ComponentName);	

	// Configure logger to log to console
	Log::initAndAddConsoleLogger(FileReplay::ComponentName, Log::MESSAGE);	
	
	// Initialize firstPublishTime
	firstPublishTime = 0;
	firstDataTime = 0;
		
	// Get archive filename
	string filename = gravityNode.getStringParam("ArchiveFilename", "archive.bin");
	
	string dpList = gravityNode.getStringParam("DataProductList", "");

	// Kick off the thread to start reading the file
	fileReader.init(filename, dpList);
	pthread_t readerThread;
	pthread_create(&readerThread, NULL, &FileReader::start, &fileReader);
		
	// Start processing the archive file
	processArchive();
}

FileReplay::~FileReplay() {}

void FileReplay::processArchive()
{
    int size;    
  
    shared_ptr<GravityDataProduct> gdp = fileReader.getNextDataProduct();
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
        uint64_t timestamp = gdp->getGravityTimestamp();
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
				Log::debug("waiting %d", timeToWait-elapsedTime);
#ifdef WIN32
				gravity::sleep( (timeToWait-elapsedTime) / 1000 );
#else
                usleep(timeToWait - elapsedTime);
#endif
            }                       
        }        
    
        // publish
        Log::debug("publishing %s", gdp->getDataProductID().c_str());
        gravityNode.publish(*gdp);        
   
	// Get the next one 
	gdp = fileReader.getNextDataProduct();
    }
        
    Log::message("Reached end of archive");
}

void FileReplay::waitForExit()
{
	gravityNode.waitForExit();
}

} /* namespace esmf */
