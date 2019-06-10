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
 *  Created on: Sept. 19, 2012
 *      Author: Tim Ludwinski
 */

#ifndef GRAVITYHEARTBEATLISTENER_H_
#define GRAVITYHEARTBEATLISTENER_H_

#include <string>
#include <stdint.h>
#include "Utility.h"

namespace gravity
{

/**
 * Interface specification for an object that will respond to connection outages of gravity products.
 */
class GravityHeartbeatListener
{
public:
	/**
	 * Called when another component's heartbeat is off by a certain amount.
	 * \param componentID component id for the component whose heartbeat was missed
	 * \param microsecond_to_last_heartbeat number of microseconds since the last heartbeat was received, or -1 if
	 * a heart beat was never received.  Typed as a signed 64 bit integer to make passing to Java via Swig cleaner.
	 * \param interval_in_microseconds The current heart beat interval for the given component ID.  Updates to this value
	 * will change the interval used in subsequent iterations.  Typed as a signed 64 bit integer to
     * make passing to Java via Swig cleaner.
	 */
	GRAVITY_API virtual void MissedHeartbeat(std::string componentID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds) = 0;

	/**
	 * Called when another component's heartbeat is received
     * \param componentID component id for the component whose heartbeat was received
     * \param interval_in_microseconds The current heart beat interval for the given component ID.  Updates to this value
     * will change the interval used in subsequent iterations.  Typed as a signed 64 bit integer to
     * make passing to Java via Swig cleaner.
	 */
	GRAVITY_API virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds) = 0;

    /**
     * Default destructor
     */
	GRAVITY_API virtual ~GravityHeartbeatListener() { };
};

} /* namespace gravity */
#endif //GRAVITYHEARTBEATLISTENER_H_
