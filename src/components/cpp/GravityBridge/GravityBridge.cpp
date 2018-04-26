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

#include "GravityBridge.h"

#include "GravityLogger.h"

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
namespace gp = google::protobuf;

#include <MOOS/libMOOS/Comms/MOOSCommClient.hxx>

#include <fcntl.h>
#ifndef WIN32
#include <sys/unistd.h>
#endif

using namespace std;
using namespace std::tr1;

int main(int argc, const char* argv[]) {
    gravity::GravityBridge bridge;
    return bridge.run();
}

namespace gravity {

const char* GravityBridge::ComponentName = "GravityBridge";

bool wrap_MOOSVoidFunc(void *func) {
    if (!func) { return false; }
    return (*static_cast<MOOSVoidFunc*>(func))();
}

GravityBridge::GravityBridge() {
    // Initialize Gravity Node
    _gravityNode.init(GravityBridge::ComponentName);
    
    // Configure logger to log to console
    Log::initAndAddConsoleLogger(GravityBridge::ComponentName, Log::MESSAGE);
   
    // Parse Protobuf Config file
    int cfg_fd = open(_gravityNode.getStringParam("config", "bridge.pbtxt").c_str(), O_RDONLY);
    gp::io::FileInputStream cfg_reader(cfg_fd);
    gp::TextFormat::Parse(&cfg_reader, &_cfg);
    close(cfg_fd);
    
    // Setup MOOS connected callback and initialize MOOS connection.
    _connectedCb = 
        shared_ptr<MOOSVoidFunc>(new MOOSVoidFunc(bind(&GravityBridge::moosConnected, this)));
    _moosComm.SetOnConnectCallBack(wrap_MOOSVoidFunc, _connectedCb.get());
    
    // Setup MOOSPB/Gravity registrations/subscriptions/callbacks.  Gravity will do the right thing
    // across a server restart, but MOOS will not.  MOOS sub's are therefore in ::moosConnected().
    for(int i=0, n=_cfg.gravity_to_moospb_size(); i < n; ++i) {
        std::string pubname = _cfg.gravity_to_moospb(i);
        Log::debug("Subscribing to gravity publication '%s'", pubname.c_str());
        _gravityNode.subscribe(pubname, *this);
    }
    for(int i=0, n=_cfg.moospb_to_gravity_size(); i < n; ++i) {
        std::string pubname = _cfg.moospb_to_gravity(i);
        std::string gravity_name = _cfg.gravity_prefix() + pubname;
        Log::debug("Registering gravity publication '%s'", gravity_name.c_str());
        _gravityNode.registerDataProduct(gravity_name, GravityTransportTypes::TCP);
        
        _moosComm.AddMessageRouteToActiveQueue<GravityBridge>(pubname + " Queue", pubname,
            this, &GravityBridge::moosMessageReceived);

    }
    _moosComm.Run(_cfg.moos_server(), _cfg.moos_port(), "GravityBridge", _cfg.moos_frequency());
}

bool GravityBridge::moosConnected() {
    // Setup MOOS subscriptions - MOOS doesn't register on the publication side until Notify()
    for(int i=0, n=_cfg.moospb_to_gravity_size(); i < n; ++i) {
        Log::debug("Subscribing to MOOS publication '%s'", _cfg.moospb_to_gravity(i).c_str());
        _moosComm.Register(_cfg.moospb_to_gravity(i));
    }
    return true;
}

bool GravityBridge::moosMessageReceived(CMOOSMsg& msg) {
    // Publish this received MOOS message on the Gravity side.
    std::string gravity_name = _cfg.gravity_prefix() + msg.GetKey();
    std::string gravity_component = _cfg.gravity_prefix() + msg.GetSource();
    int64_t gravity_time = msg.GetTime() * 1000000L;
    GravityDataProduct gdp(gravity_name);
    std::string sample_data = "Gravity Test Data";
    gdp.setData(sample_data.c_str(), sample_data.size());
    gdp.setTimestamp(gravity_time);
    gdp.setComponentId(gravity_component);
    gdp.setDomain("MOOS");
    gdp.setIsRelayedDataproduct(true);
    if (_gravityNode.publish(gdp) != GravityReturnCodes::SUCCESS) {
        Log::warning("Unable to publish '%s' via Gravity.", gravity_name.c_str());
    };
    return true;
}

void GravityBridge::subscriptionFilled(const DataProductVec& dataProducts) {
    Log::trace("Received %u GravityDataProducts", (unsigned)dataProducts.size());
    for (DataProductVec::const_iterator it = dataProducts.begin();
        it != dataProducts.end(); ++it) {
        if (_moosComm.IsConnected()) {
            std::string moos_name = _cfg.moospb_prefix() + (*it)->getDataProductID();
            std::string moos_source = _cfg.moospb_prefix() + (*it)->getComponentId();
            double moos_time = (*it)->getGravityTimestamp() / 1000000.0;
            // TODO: Converting protobuf from binary to text means needing types... ugh.
            std::string sample_data = "MOOS Test Data";
            CMOOSMsg msg(MOOS_NOTIFY, moos_name, sample_data.c_str(), moos_time);
            msg.SetSource(moos_source);
            _moosComm.Post(msg, true); // true: preserve Source Name
        }
    }    
}

void GravityBridge::sendHeartbeats() {
    _moosComm.Notify("GravityBridgeTick", "");
    GravityDataProduct gdp("GravityBridgeTick");
    _gravityNode.publish(gdp);
}

GravityBridge::~GravityBridge() { }

int GravityBridge::run() {
    // Send heartbeats to both universes at 0.5Hz.
    while (true) {
        sendHeartbeats();
        usleep(2000000);
    }
    
    _gravityNode.waitForExit();
    return 0;
}

} /* namespace gravity */
