/*
 * ServiceDirectory.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#ifndef SERVICEDIRECTORY_H_
#define SERVICEDIRECTORY_H_

#include <string>
#include <map>

using namespace std;

namespace gravity
{

class ServiceDirectory
{
private:
	map<std::string,std::string> dataProductMap;
	map<std::string,std::string> serviceMap;
public:
	ServiceDirectory();
	virtual ~ServiceDirectory();
	void start();
};

} /* namespace gravity */
#endif /* SERVICEDIRECTORY_H_ */
