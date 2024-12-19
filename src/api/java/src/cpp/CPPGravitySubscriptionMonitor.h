/** (C) Copyright 2016, Applied Physical Sciences Corp., A General Dynamics Company
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

#ifndef CPPGRAVITYSUBSCRIPTIONMONITOR_H_
#define CPPGRAVITYSUBSCRIPTIONMONITOR_H_

#include "GravitySubscriptionMonitor.h"

namespace gravity
{

class CPPGravitySubscriptionMonitor : public GravitySubscriptionMonitor
{
public:
	virtual ~CPPGravitySubscriptionMonitor();
	virtual void subscriptionTimeout(std::string dataProductID, int milliSecondsSinceLast, std::string filter, std::string domain);
	virtual void subscriptionTimeoutJava(const std::string& dataProductID, int milliSecondsSinceLast, const std::string& filter, const std::string& domain);
};
}

#endif /* CPPGRAVITYSUBSCRIPTIONMONITOR_H_ */