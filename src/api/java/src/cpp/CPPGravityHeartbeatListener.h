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

#ifndef CPPGRAVITYHEARTBEATLISTENER_H_
#define CPPGRAVITYHEARTBEATLISTENER_H_

#include "GravityHeartbeatListener.h"

namespace gravity
{

/**
 * Native implementation of a GravityHeartbeatListener
 */
class CPPGravityHeartbeatListener : public GravityHeartbeatListener
{
public:
    virtual ~CPPGravityHeartbeatListener();
    virtual void MissedHeartbeat(std::string componentID, int64_t microsecond_to_last_heartbeat,
                                 int64_t& interval_in_microseconds);
    virtual int64_t MissedHeartbeatJava(const std::string componentID, int64_t microsecond_to_last_heartbeat,
                                        int64_t& interval_in_microseconds);
    virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds);
    virtual int64_t ReceivedHeartbeatJava(const std::string componentID, int64_t& interval_in_microseconds);
};

}  // namespace gravity
#endif /* CPPGRAVITYHEARTBEATLISTENER_H_ */
