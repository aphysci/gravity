/*
 * ServiceDirectory.h
 *
 *  Created on: Jun 28, 2012
 *      Author: esmf
 */

#ifndef SERVICEDIRECTORY_H_
#define SERVICEDIRECTORY_H_

#include <map>
#include <vector>

#include <DataProductDescription.h>

using namespace std;

namespace esmf
{

class ServiceDirectory
{
private:
	map<int, vector<DataProductDescription> > registrationMap;
public:
	ServiceDirectory();
	virtual ~ServiceDirectory();
	void start();
};

} /* namespace esmf */
#endif /* SERVICEDIRECTORY_H_ */
