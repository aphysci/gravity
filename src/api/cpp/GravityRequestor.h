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
 * GravityRequestor.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYREQUESTOR_H_
#define GRAVITYREQUESTOR_H_

#include "GravityDataProduct.h"

namespace gravity
{

/**
 * Interface specification for an object that will function as the "client" side of a request-response interaction
 */
class GravityRequestor
{
public:
    /**
     * Default destructor
     */
    GRAVITY_API virtual ~GravityRequestor();

    /**
     * Called when a response to a request is received through the Gravity infrastructure
     * \param serviceID ID of the service request is being filled through
     * \param requestID ID of the request that was made
     * \param response GravityDataProduct containing the data of the response
     */
    GRAVITY_API virtual void requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response) = 0;

    /**
     * Called when the response to a request has timed out. 
     * \param serviceID ID of the service request is being filled through
     * \param requestID ID of the request that was made
     */
    GRAVITY_API virtual void requestTimeout(std::string serviceID, std::string requestID);
};

} /* namespace gravity */


#endif /* GRAVITYREQUESTOR_H_ */
