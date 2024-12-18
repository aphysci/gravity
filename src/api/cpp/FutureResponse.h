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
 * FutureResponse.h
 *
 *  Created on: April 26, 2016
 *      Author: Chris Brundick
 */

#ifndef FUTURERESPONSE_H_
#define FUTURERESPONSE_H_

#include "GravityDataProduct.h"
#include "GravityNode.h"
#include "Utility.h"

namespace gravity
{

/**
 * Representative of a future response to a gravity request
 */
class FutureResponse : public GravityDataProduct
{
private:
    friend class GravityNode;

    /**
	 * Private constructor
	 */
    FutureResponse(std::string url);

    /**
   * Get socket URL
   */
    std::string getUrl() const;

public:
    /**
     * Constructor that deserializes this FutureResponse from array of bytes
     * \param arrayPtr pointer to array of bytes containing serialized FutureResponse
     * \param size size of serialized data
     * \return a FutureResponse
     */
    GRAVITY_API FutureResponse(const void* arrayPtr, int size);

    /**
     * Default Destructor
     */
    GRAVITY_API virtual ~FutureResponse();

    /**
	 * Method for setting the GravityDataProduct response object to be returned to requestor
	 */
    GRAVITY_API void setResponse(const GravityDataProduct& response);
};

} /* namespace gravity */
#endif /* FUTURERESPONSE_H_ */
