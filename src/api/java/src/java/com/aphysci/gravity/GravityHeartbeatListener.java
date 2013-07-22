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

package com.aphysci.gravity;

public interface GravityHeartbeatListener {
	/**
	 * 
	 * @param componentID id of the component whose heart beat has not been received
	 * @param microsecond_to_last_heartbeat microseconds since the last heart beat was received
	 * from the given component, or -1 if a heart beat was never received.    
	 * @param interval_in_microseconds an array of length 1 that contains the current interval at which
	 * a heart beat is expected.  Updates to this value will change the interval used in subsequent iterations.
	 */
	void MissedHeartbeat(String componentID, long microsecond_to_last_heartbeat, long[] interval_in_microseconds);
	
	/**
	 * 
	 * @param componentID id of the component whose heart beat has been received
	 * @param interval_in_microseconds an array of length 1 that contains the current interval at which
	 * a heart beat is expected.  Updates to this value will change the interval used in subsequent iterations.
	 */
	void ReceivedHeartbeat(String componentID, long[] interval_in_microseconds);
}
