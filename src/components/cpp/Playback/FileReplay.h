/*
 * FileReplay.h
 *
 *  Created on: Jan 16, 2013
 *      Author: esmf
 */

#ifndef FILEREPLAY_H_
#define FILEREPLAY_H_

#include "GravityNode.h"
#include <fstream>
#include <set>

namespace esmf {

using namespace gravity;
using namespace std;

class FileReplay
{
private:	
	static const char* ComponentName;
	uint64_t firstPublishTime;
	uint64_t firstDataTime;

	FileReader fileReader;
    
	GravityNode gravityNode;	
	set<string> datatypes;
	
	void processArchive();
public:
	FileReplay();
	virtual ~FileReplay();
	void waitForExit();	
};

} /* namespace esmf */
#endif /* FILEREPLAY_H_ */
