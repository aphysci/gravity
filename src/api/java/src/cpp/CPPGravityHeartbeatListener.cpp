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

#include "CPPGravityHeartbeatListener.h"
#include <iostream>

using namespace gravity;
using namespace std;

CPPGravityHeartbeatListener::~CPPGravityHeartbeatListener() {}

void CPPGravityHeartbeatListener::MissedHeartbeat(std::string componentID, int64_t microsecond_to_last_heartbeat,
                                                  int64_t& interval_in_microseconds)
{
    MissedHeartbeatJava(componentID, microsecond_to_last_heartbeat, interval_in_microseconds);
}

int64_t CPPGravityHeartbeatListener::MissedHeartbeatJava(const std::string componentID,
                                                         int64_t microsecond_to_last_heartbeat,
                                                         int64_t& interval_in_microseconds)
{
    return 0L;
}

void CPPGravityHeartbeatListener::ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds)
{
    ReceivedHeartbeatJava(componentID, interval_in_microseconds);
}

int64_t CPPGravityHeartbeatListener::ReceivedHeartbeatJava(const std::string componentID,
                                                           int64_t& interval_in_microseconds)
{
    return 0L;
}
