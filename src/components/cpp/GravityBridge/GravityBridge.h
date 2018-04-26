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

#ifndef _GRAVITY__GRAVITYBRIDGE_H_
#define _GRAVITY__GRAVITYBRIDGE_H_

#include "GravityNode.h"
#include "GravitySubscriber.h"
#include "protobufs/GravityBridgeConfig.pb.h"

#include <MOOS/libMOOS/Comms/MOOSAsyncCommClient.h>

#include <set>
#include <string>
#include <tr1/functional>
#include <tr1/memory>

namespace gravity {

typedef std::tr1::function<bool()> MOOSVoidFunc;
typedef std::vector<std::tr1::shared_ptr<GravityDataProduct> > DataProductVec;

class GravityBridge : public GravitySubscriber {
public:
    GravityBridge();
    virtual ~GravityBridge();

    int run();
    
protected:
    static const char *ComponentName;

    // Sets of all the publications that can be republished simply by converting PB to Gravity.
    std::set<std::string> _moospbToRepublish;
    std::set<std::string> _gravityToRepublish;

    GravityBridgeConfig _cfg;
    GravityNode _gravityNode;
    MOOS::MOOSAsyncCommClient _moosComm;

    void subscriptionFilled(const DataProductVec& dataProducts);
    bool moosMessageReceived(CMOOSMsg& msg);

    bool moosConnected();

    void sendHeartbeats();
    
    // MOOS Connected Callback Wrapper
    std::tr1::shared_ptr<MOOSVoidFunc> _connectedCb;
};

} /* namespace gravity */
#endif /* _GRAVITY__GRAVITYBRIDGE_H_ */
