/*
 * FileReader.h
 *
 *  Created on: Feb 13, 2013
 *      Author: esmf
 */

#ifndef FILEREADER_H_
#define FILEREADER_H_

#include <GravityDataProduct.h>
#include <pthread.h>
#include <fstream>
#include <vector>

namespace esmf {

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

} /* namespace esmf */
#endif /* FILEREADER_H_ */
