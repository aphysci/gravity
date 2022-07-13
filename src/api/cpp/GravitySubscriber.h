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

/*
 * GravitySubscriber.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSUBSCRIBER_H_
#define GRAVITYSUBSCRIBER_H_

#include "GravityDataProduct.h"
#include "protobuf/GravityMetricsDataPB.pb.h"

using namespace std;

namespace gravity
{

/**
 * Interface specification for an object that will respond to subscriptions
 */
class GravitySubscriber
{
public:
    /**
     * Default destructor
     */
    GRAVITY_API virtual ~GravitySubscriber();

    /**
     * Called on implementing object when a registered subscription is filled with 1 or more GravityDataProducts
     * \param dataProducts the data products that fill the registered subscription
     */
    GRAVITY_API virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts) = 0;
};

class GravityMetricsSubscriber : public GravitySubscriber {

    // GRAVITY_API ~GravitySubscriber() override;

public:
    
    virtual void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts) {
		// for(std::vector< shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
        // {
        //     //Get the protobuf object from the message
        //     // GravityMetricsDataPB metricsPB;
        //     // (*i)->populateMessage(metricsPB);

        //     //Process the message
        //     // Log::warning("Current Count: %d", metricsPB.count());
        // }
	}
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIBER_H_ */
