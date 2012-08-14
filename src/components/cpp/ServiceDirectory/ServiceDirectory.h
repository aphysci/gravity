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

using namespace std;

namespace esmf
{

class ServiceDirectory
{
private:
	map<string, string> registrationMap;
public:
	ServiceDirectory();
	virtual ~ServiceDirectory();
	void start();
};

} /* namespace esmf */
#endif /* SERVICEDIRECTORY_H_ */
