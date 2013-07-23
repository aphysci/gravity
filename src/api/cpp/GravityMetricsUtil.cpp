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
 * GravityMetricsUtil.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: Chris Brundick
 */

#include "GravityMetricsUtil.h"

namespace gravity
{

const char* GRAVITY_METRICS_CONTROL = "inproc://gravity_metrics_control";
const char* GRAVITY_PUB_METRICS_REQ = "inproc://gravity_pub_metrics_request";
const char* GRAVITY_SUB_METRICS_REQ = "inproc://gravity_sub_metrics_request";
const char* GRAVITY_METRICS_PUB = "inproc://gravity_metrics_pub";
const std::string GRAVITY_METRICS_DATA_PRODUCT_ID = "GravityMetricsData";

} /* namespace gravity */
