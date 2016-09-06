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

/*
 * GravitySubscriptionMonitor.h
 *
 *  Created on: Aug 11, 2016
 *      Author: Joseph Hankin
 */

#ifndef GRAVITYSUBSCRIPTIONMONITOR_H_
#define GRAVITYSUBSCRIPTIONMONITOR_H_

#include "Utility.h"

namespace gravity
{

/**
 * Interface specification for an object that will respond to subscriptions
 */
class GravitySubscriptionMonitor
{
public:
    /**
     * Default destructor
     */
    GRAVITY_API virtual ~GravitySubscriptionMonitor();

    /**
     * Called on implementing object when a subscription is not received within the registered time constraints
     * \param dataProductID the name of the data product for the subscription
	 * \param milliSecondsSinceLast the time of the last received data product, or -1 if never received.
	 * \param filter the name of the filter registered for this data product
	 * \param domain the name of the domain for this data product
     */
	virtual void subscriptionTimeout(std::string dataProductID, int milliSecondsSinceLast, std::string filter, std::string domain) = 0;
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIPTIONMONITOR_H_ */
