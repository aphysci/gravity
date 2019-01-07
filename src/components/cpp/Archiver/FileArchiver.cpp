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
#include "FileArchiver.h"
#include <sstream>
#include <iostream>
#include <string>

#include "protobuf/FileArchiverControlRequestPB.pb.h"
#include "protobuf/FileArchiverControlResponsePB.pb.h"

using namespace std;

int main(int argc, const char* argv[])
{
	gravity::FileArchiver archiver;
	archiver.waitForExit();
}

namespace gravity {

const char* FileArchiver::ComponentName = "FileArchiver";

FileArchiver::FileArchiver()
{
	// Initialize Gravity Node
	gravityNode.init(FileArchiver::ComponentName);

	// Configure logger to log to console
	Log::initAndAddConsoleLogger(FileArchiver::ComponentName, Log::MESSAGE);

	// Open archive file
	string filename = gravityNode.getStringParam("ArchiveFilename", "archive.bin");
	if (filename == "auto")
	{
		time_t now;
		struct tm ts;
		char fname[80];
		time(&now);
		ts = *localtime(&now);
		strftime(fname, sizeof(fname), "archive_%Y%m%d_%H%M%S.bin", &ts);
		//cout << "fname = '" << fname << "'" << endl;
		archiveFile.open(fname, ios::out | ios::binary);
	}
	else
	{
		archiveFile.open(filename.c_str(), ios::out | ios::binary);
	}

	// Write value (1) to endian-ness check when read
	int value = 1;
        archiveFile.write((char*)&value, sizeof(value));

	// Get list of data products to archive
	string dpList = gravityNode.getStringParam("DataProductList", "");

	// allow starting suspended
	suspend = gravityNode.getBoolParam("StartSuspended", false);

	gravityNode.registerService("FileArchiverControlRequest", GravityTransportTypes::TCP, *this);

	// Subscribe to each data product
	vector<string> dps = split(dpList);
	for (vector<string>::iterator iter = dps.begin(); iter != dps.end(); iter++)
	{
	    gravityNode.subscribe(*iter, *this);
	}
}

FileArchiver::~FileArchiver()
{
}

vector<string> FileArchiver::split(string s)
{
	vector<string> tokens;

	std::istringstream iss(s);
	std::string tok;
	while (getline(iss, tok, ','))
	{
		// trim any spaces from the ends
		tok.erase(tok.find_last_not_of(" ") + 1);
		tok.erase(0, tok.find_first_not_of(" "));
		tokens.push_back(tok);
	}

	return tokens;
}

void FileArchiver::subscriptionFilled(const vector<tr1::shared_ptr<GravityDataProduct> >& dataProducts)
{
    if (!suspend)
    {
        for (unsigned int i = 0; i < dataProducts.size(); i++)
        {
            tr1::shared_ptr<GravityDataProduct> dataProduct = dataProducts.at(i);

            // Write the size of the data product
            int size = dataProduct->getSize();
            archiveFile.write((char*)&size, sizeof(size));

            // Write the data product to the archive file
            std::vector<char> buffer(size);
            dataProduct->serializeToArray(&buffer[0]);
            archiveFile.write(buffer.data(), size);

            // Flush file
            archiveFile.flush();
        }
    }
}

tr1::shared_ptr<GravityDataProduct> FileArchiver::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
    Log::debug("Received service request of type '%s'", serviceID.c_str());
    FileArchiverControlResponsePB faResponse;
    faResponse.set_status(false);

    if (serviceID == "FileArchiverControlRequest")
    {
        FileArchiverControlRequestPB faRequest;
        if (!dataProduct.populateMessage(faRequest))
        {
            Log::critical("Unable to deserialize received FileArchiverControlRequest data");
            faResponse.set_status(false);
        }
        else
        {
            suspend = faRequest.suspend();
            Log::warning("Suspend flag has been set to %s", suspend ? "true" : "false");
            faResponse.set_status(true);
        }
    }

    tr1::shared_ptr<GravityDataProduct> gdpResponse = tr1::shared_ptr<GravityDataProduct>(new GravityDataProduct("FileArchiverControlResponse"));
    gdpResponse->setData(faResponse);
    return gdpResponse;
}

void FileArchiver::waitForExit()
{
	gravityNode.waitForExit();
}

} /* namespace gravity */
