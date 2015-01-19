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
 * GravityServiceProvider.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYSERVICEPROVIDER_H_
#define GRAVITYSERVICEPROVIDER_H_

#include "GravityDataProduct.h"

namespace gravity
{

/**
 * Interface specification for an object that will function as the "server" side of a request-response interaction
 */
class GravityServiceProvider
{
public:
    /**
     * Default destructor
     */
    GRAVITY_API virtual ~GravityServiceProvider();

    /**
     * Called when a request is made through the Gravity infrastructure
     * \param serviceID service ID of the requesting service
     * \param dataProduct GravityDataProduct with request data
     * \returns the response
     */
    virtual shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct) = 0;
};

} /* namespace gravity */


#endif /* GRAVITYSERVICEPROVIDER_H_ */
