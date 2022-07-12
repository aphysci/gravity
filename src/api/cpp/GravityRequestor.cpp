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
 * GravityRequestor.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: Chris Brundick
 */

#include "GravityRequestor.h"
#include "GravityLogger.h"
#include "spdlog/spdlog.h"

namespace gravity {

using namespace std;

GRAVITY_API GravityRequestor::~GravityRequestor() {}

GRAVITY_API void GravityRequestor::requestTimeout(std::string serviceID, std::string requestID)
{
    spdlog::get("GravityLogger")->warn("Request timed out: service id = {}, request id = {}", serviceID, requestID);
}

}




