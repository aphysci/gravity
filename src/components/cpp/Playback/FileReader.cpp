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

#include "FileReader.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#ifndef WIN32
#include <sys/unistd.h>
#endif

using namespace std;

namespace gravity {

FileReader::FileReader() {}

void FileReader::init(const string& filename, const string& dataProductList)
{
	// Initialize mutex
	pthread_mutex_init(&mutex, NULL);

	// Open archive file
	archiveFile.open(filename.c_str(), ios::in | ios::binary);

	// Check endian-ness
	int i;
	archiveFile.read((char*)&i, sizeof(i));
	swapEndian = (i != 1);

	// Build list of data products of interest
	dpList = split(dataProductList);
}

FileReader::~FileReader() {}

void* FileReader::start(void* context)
{
	((FileReader*)context)->processArchive();
	return NULL;
}

void FileReader::processArchive()
{
    while (readNextDataProduct())
    {
	pthread_mutex_lock(&mutex);
	while (archiveFile && dataProducts.size() > 100)
	{
		pthread_mutex_unlock(&mutex);
#ifdef WIN32
		gravity::sleep(10);
#else
		usleep(10000);
#endif
		pthread_mutex_lock(&mutex);
	}
	pthread_mutex_unlock(&mutex);
    }
}

int FileReader::readNextDataProduct()
{
	int size;
	pthread_mutex_lock(&mutex);

	while (archiveFile)
	{
        	// Read the size of the next data product
        	archiveFile.read((char*)&size, sizeof(size));
        	if (!archiveFile)
        	{
            		// Likely we've reached the end of file.
            		break;
        	}

		if (swapEndian)
		{
			endian_swap(size);
		}
        	// Read the data product
        	std::vector<char> buffer(size);
        	archiveFile.read(&buffer[0], size);
        	if (!archiveFile)
        	{
            		// Treat this as end of file
            		break;
        	}

        	// Convert to GravityDataProduct
        tr1::shared_ptr<GravityDataProduct> gdp = tr1::shared_ptr<GravityDataProduct>(new GravityDataProduct(buffer.data(), size));
		//gdp->parseFromArray(buffer, size);

		// Make sure this is a data product we care about
		if (!dpList.empty() && find(dpList.begin(), dpList.end(), gdp->getDataProductID()) == dpList.end())
		{
			continue;
		}

		dataProducts.push_back(gdp);
		break;
	}

	int sz = dataProducts.size();
	pthread_mutex_unlock(&mutex);

	return sz;
}

vector<string> FileReader::split(string s)
{
        vector<string> tokens;

        istringstream iss(s);
        string tok;
        while (getline(iss, tok, ','))
        {
                // trim any spaces from the ends
                tok.erase(tok.find_last_not_of(" ") + 1);
                tok.erase(0, tok.find_first_not_of(" "));
                tokens.push_back(tok);
        }

        return tokens;
}

bool FileReader::hasData()
{
	pthread_mutex_lock(&mutex);
	bool ret = !dataProducts.empty();
	pthread_mutex_unlock(&mutex);
	return ret;
}

tr1::shared_ptr<GravityDataProduct> FileReader::popGravityDataProduct()
{
	pthread_mutex_lock(&mutex);
	tr1::shared_ptr<GravityDataProduct> gdp = dataProducts[0];
	dataProducts.erase(dataProducts.begin());
	pthread_mutex_unlock(&mutex);
	return gdp;
}

tr1::shared_ptr<GravityDataProduct> FileReader::getNextDataProduct()
{
    tr1::shared_ptr<GravityDataProduct> gdp;
	if (!hasData() && !readNextDataProduct())
	{
	}
	else
	{
		gdp = popGravityDataProduct();
	}
	return gdp;
}

void FileReader::endian_swap(int& i)
{
	i = (i >> 24) | ((i<<8)&0x00FF0000) | ((i>>8) & 0x0000FF00) | (i << 24);
}

} /* namespace gravity */
