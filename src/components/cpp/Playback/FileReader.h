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

#ifndef FILEREADER_H_
#define FILEREADER_H_

#include <GravityDataProduct.h>
#include <pthread.h>
#include <fstream>
#include <vector>

namespace gravity {

using namespace gravity;
using namespace std;

class FileReader
{
private:
	ifstream archiveFile;
	vector<shared_ptr<GravityDataProduct> > dataProducts;
	vector<string> dpList;
	pthread_mutex_t mutex;
	void processArchive();
	shared_ptr<GravityDataProduct> popGravityDataProduct();
	bool hasData();
	vector<string> split(string s);
	int readNextDataProduct();

	bool swapEndian;
	void endian_swap(int& i);
public:
	FileReader();
	void init(const string& filename, const string& dataProductList);
	virtual ~FileReader();
	static void* start(void* context);
	shared_ptr<GravityDataProduct> getNextDataProduct();
};

} /* namespace gravity */
#endif /* FILEREADER_H_ */
