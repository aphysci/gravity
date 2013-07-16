/*
 * FileArchiver.h
 *
 *  Created on: Jan 16, 2013
 *      Author: esmf
 */

#ifndef FILEARCHIVER_H_
#define FILEARCHIVER_H_

#include "GravityNode.h"
#include <string>
#include <fstream>
#include <vector>

namespace esmf {

using namespace gravity;
using namespace std;

class FileArchiver : public GravitySubscriber
{
private:	
	static const char* ComponentName;
	GravityNode gravityNode;	
	ofstream archiveFile;
	
	vector<string> split(string s);	
public:
	FileArchiver();
	virtual ~FileArchiver();

	virtual void subscriptionFilled(const vector<shared_ptr<GravityDataProduct> >& dataProducts);
	void waitForExit();	
};

} /* namespace esmf */
#endif /* FILEARCHIVER_H_ */
