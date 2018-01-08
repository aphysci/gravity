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
    virtual void subscriptionFilled(const std::vector< std::tr1::shared_ptr<GravityDataProduct> >& dataProducts) = 0;
};

} /* namespace gravity */
#endif /* GRAVITYSUBSCRIBER_H_ */
