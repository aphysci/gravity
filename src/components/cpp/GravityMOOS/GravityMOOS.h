/** (C) Copyright 2018, Applied Physical Sciences Corp., A General Dynamics Company
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

#ifndef _GRAVITY__GRAVITY_MOOS_BRIDGE_H_
#define _GRAVITY__GRAVITY_MOOS_BRIDGE_H_

#include "GravityNode.h"
#include "GravitySubscriber.h"
#include "protobufs/GravityMOOSConfig.pb.h"
#include "ProtobufUtilities.h"

#include <MOOS/libMOOS/Comms/MOOSAsyncCommClient.h>

#include <set>
#include <string>
#include <tr1/memory>

#include <cstdio>

namespace gravity {

typedef std::vector<std::tr1::shared_ptr<GravityDataProduct> > DataProductVec;

class GravityMOOS : public GravitySubscriber {
public:
    GravityMOOS();
    virtual ~GravityMOOS();
    
    int run();
    
    // public to work around MOOS's inadequacies...
    bool moosConnected();
        
protected:
    static const char *ComponentName;

    protobuf::GravityMOOSConfig _cfg;
    ProtobufRegistry _registry;
    
    GravityNode _node;
    void subscriptionFilled(const DataProductVec& gdp);

    MOOS::MOOSAsyncCommClient _moosComm;
    bool moosMessageReceived(CMOOSMsg& msg);

    void sendHeartbeat();
};

} /* namespace gravity */
#endif /* _GRAVITY__GRAVITY_MOOS_BRIDGE_H_ */
