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

#include "GravityMOOS.h"
#include "GravityLogger.h"
#include "ProtobufUtilities.h"

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
namespace gp = google::protobuf;

#include <fcntl.h>
#ifndef WIN32
#include <sys/unistd.h>
#endif

using namespace std;
using namespace std::tr1;

namespace gravity {

const char* GravityMOOS::ComponentName = "GravityMOOS";

bool wrapMoosConnected(void *param) {
    return static_cast<GravityMOOS*>(param)->moosConnected();
}

GravityMOOS::GravityMOOS() {
    // Initialize Gravity Node
    _gravityNode.init(GravityMOOS::ComponentName);
    
    // Configure logger to log to console
    Log::initAndAddConsoleLogger(GravityMOOS::ComponentName, Log::MESSAGE);
   
    // Parse Protobuf Config file
    bool success = parseTextPB(_gravityNode.getStringParam("config", "gravitymoos.pbtxt").c_str(), _cfg);
    if (!success) { throw std::runtime_error("Could not parse configuration file!"); }
    
    // Setup MOOSPB/Gravity registrations/subscriptions/callbacks.  Gravity will do the right thing
    // across a server restart, but MOOS will not.  We can setup the MOOS callback here, but the
    // subscriptions are made in moosConnected().
    for(int i=0, n=_cfg.gravity_publications_size(); i < n; ++i) {
        Log::debug("Subscribing to gravity publication '%s'", _cfg.gravity_publications(i).c_str());
        _gravityNode.subscribe(_cfg.gravity_publications(i), *this);
    }
    for(int i=0, n=_cfg.moospbt_publications_size(); i < n; ++i) {
        std::string pubname = _cfg.moospbt_publications(i);
        Log::debug("Registering gravity publication '%s'", pubname);
        _gravityNode.registerDataProduct(pubname, GravityTransportTypes::TCP);
        _moosComm.AddMessageRouteToActiveQueue<GravityMOOS>(pubname + " Queue", pubname,
            this, &GravityMOOS::moosMessageReceived);
    }

    // Setup onConnected callback and kick the MOOS.
    _moosComm.SetOnConnectCallBack(wrapMoosConnected, this);
    _moosComm.Run(_cfg.moos_server(), _cfg.moos_port(), "GravityBridge");
}

GravityMOOS::~GravityMOOS() { }

int GravityMOOS::run() {
    // Send periodic heartbeat to each universe, but mostly sleep and let the translators work.
    const static long MICROSECS_TO_SLEEP = 1000000.0 / _cfg.heartbeat_frequency();
    while (true) {
        sendHeartbeat();
        usleep(MICROSECS_TO_SLEEP);
    }
    
    _gravityNode.waitForExit();
    return 0;
}

void GravityMOOS::sendHeartbeat() {
    _moosComm.Notify("GravityMOOSTick", "");
    GravityDataProduct gdp("GravityMOOSTick");
    _gravityNode.publish(gdp);
}

bool GravityMOOS::moosConnected() {
    // Setup MOOS subscriptions - MOOS doesn't explicitly register publications until a Notify()
    for(int i=0, n=_cfg.moospbt_publications_size(); i < n; ++i) {
        Log::debug("Subscribing to MOOS publication '%s'", _cfg.moospbt_publications(i).c_str());
        _moosComm.Register(_cfg.moospbt_publications(i));
    }
    return true;
}

bool GravityMOOS::moosMessageReceived(CMOOSMsg& msg) {
    GravityDataProduct gdp(msg.GetKey());
    gdp.setTimestamp(msg.GetTime() * 1000000L);
    gdp.setComponentId(msg.GetSource());
    gdp.setDomain(_gravityNode.getDomain()); //"MOOS"
    gdp.setIsRelayedDataproduct(true);

    // Parse the LAMSS header, and figure out if it's binary or string - set data appropriately.
    std::string data = "SAMPLEDATA";
    if (msg.IsBinary()) {
        // TODO: Converting protobuf from binary to text means needing types... ugh.
        gdp.setData(data.c_str(), data.size());
    } else if (msg.IsString()) {
        // TODO: Converting protobuf from binary to text means needing types... ugh.
        gdp.setData(data.c_str(), data.size());
    }
    
    // Finally, publish in Gravity.
    if (_gravityNode.publish(gdp) != GravityReturnCodes::SUCCESS) {
        Log::warning("Unable to publish '%s' via Gravity.", msg.GetKey().c_str());
    };
    return true;
}

void GravityMOOS::subscriptionFilled(const DataProductVec& dataProducts) {
    Log::trace("Received %u GravityDataProducts", (unsigned)dataProducts.size());
    for (DataProductVec::const_iterator it = dataProducts.begin();
        it != dataProducts.end(); ++it) {
        if (_moosComm.IsConnected()) {
            double moos_time = (*it)->getGravityTimestamp() / 1000000.0;
            // TODO: Converting protobuf from binary to text means needing types... ugh.
            std::string data = "MOOS Test Data";
            CMOOSMsg msg(MOOS_NOTIFY, (*it)->getDataProductID(), data.c_str(), moos_time);
            msg.SetSource((*it)->getComponentId());
            _moosComm.Post(msg, true); // true: preserve Source Name
        }
    }
}

} /* namespace gravity */
