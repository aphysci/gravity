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
    if (_node.init(GravityMOOS::ComponentName) != GravityReturnCodes::SUCCESS) {
        throw std::runtime_error("Could not connect to ServiceDirectory!");
    };
    
    // Configure logger to log to console
    Log::initAndAddConsoleLogger(GravityMOOS::ComponentName, Log::DEBUG);
   
    // Parse Protobuf Config file
    bool success = textFileToProtobuf("gravitymoos.pbtxt", _cfg);
    if (!success) { throw std::runtime_error("Could not parse configuration file!"); }
    
    // Register all the protobuf files in listed paths.
    _registry.setProtobufPath(_cfg.protobuf_path());
    shared_ptr<gp::Message> msg = shared_ptr<gp::Message>(_registry.createMessageByName("ssdte.ArrayGainMsgPB"));
    
    // Setup MOOSPB/Gravity registrations/subscriptions/callbacks.  Gravity will do the right thing
    // across a server restart, but MOOS will not.  We can setup the MOOS callback here, but the
    // subscriptions are made in moosConnected().
    for(int i=0, n=_cfg.gravity_publications_size(); i < n; ++i) {
        Log::debug("Subscribing to gravity publication '%s'", _cfg.gravity_publications(i).c_str());
        if (_node.subscribe(_cfg.gravity_publications(i), *this) != GravityReturnCodes::SUCCESS) {
            Log::critical("Failed to subscribe to '%s'", _cfg.gravity_publications(i).c_str());
        }
    }
    for(int i=0, n=_cfg.moospbt_publications_size(); i < n; ++i) {
        std::string pubname = _cfg.moospbt_publications(i);
        Log::debug("Registering gravity publication '%s'", pubname.c_str());
        _node.registerDataProduct(pubname, GravityTransportTypes::TCP);
        _moosComm.AddMessageRouteToActiveQueue<GravityMOOS>(pubname + " Queue", pubname,
            this, &GravityMOOS::moosMessageReceived);
    }
    _node.registerDataProduct("GravityMOOSTick", GravityTransportTypes::TCP);

    // Setup onConnected callback and kick the MOOS.
    _moosComm.SetOnConnectCallBack(wrapMoosConnected, this);
    _moosComm.Run(_cfg.moos_server(), _cfg.moos_port(), "GravityBridge");
}

GravityMOOS::~GravityMOOS() { }

int GravityMOOS::run() {
    // Send a periodic heartbeat.
    while (true) {
        sendHeartbeat();
        usleep(2000000);
    }
    
    _node.waitForExit();
    return 0;
}

void GravityMOOS::sendHeartbeat() {
    _moosComm.Notify("GravityMOOSTick", "");
    GravityDataProduct gdp("GravityMOOSTick");
    _node.publish(gdp);
}

bool GravityMOOS::moosConnected() {
    // Setup MOOS subscriptions, but MOOS doesn't explicitly register publications until a Notify()
    for(int i=0, n=_cfg.moospbt_publications_size(); i < n; ++i) {
        Log::debug("Subscribing to MOOS publication '%s'", _cfg.moospbt_publications(i).c_str());
        _moosComm.Register(_cfg.moospbt_publications(i));
    }
    return true;
}

bool GravityMOOS::moosMessageReceived(CMOOSMsg& msg) {
    shared_ptr<gp::Message> message;
    
    if (!msg.IsString()) {
        Log::critical("Non-string message received on '%s'.", msg.GetKey().c_str());
        return true;
    }
    
    // Parse the LAMSS header, and type name.
    if (msg.GetString().substr(0, 4) != "@PB[") {
        Log::critical("Message on '%s' didn't have Protobuf header!", msg.GetKey().c_str());
        return true;
    }
    size_t endOfType = msg.GetString().find("]", 0);
    if (endOfType == string::npos) {
        Log::critical("Malformed protobuf header on '%s'!", msg.GetKey().c_str());
        return true;
    }
    std::string typeName = msg.GetString().substr(4, endOfType-4);
    
    // Parse text format into a protobuf message.
    try {
        message = _registry.createMessageByName(typeName);
    } catch (runtime_error &e) {
        Log::critical("Couldn't create a '%s' on '%s.", typeName.c_str(), msg.GetKey().c_str());
        return true;
    }
    if (!textToProtobuf(msg.GetString().substr(endOfType+1), *message, typeName)) {
        Log::critical("Failed to parse '%s' on '%s'!", typeName.c_str(), msg.GetKey().c_str());
        return true;
    };

    // Finally, populate GravityDataProduct and publish in Gravity.
    GravityDataProduct gdp(msg.GetKey());
    gdp.setData(*message);
    gdp.setTimestamp(msg.GetTime() * 1000000L);
    gdp.setComponentId(msg.GetSource());
    gdp.setTypeName(typeName);
    gdp.setDomain(_node.getDomain());
    gdp.setIsRelayedDataproduct(true);
    
    if (_node.publish(gdp) != GravityReturnCodes::SUCCESS) {
        Log::warning("Unable to publish '%s' via Gravity.", msg.GetKey().c_str());
    };
    return true;
}

void GravityMOOS::subscriptionFilled(const DataProductVec& dataProducts) {
    Log::trace("Received %u GravityDataProducts", (unsigned)dataProducts.size());
    if (!_moosComm.IsConnected()) {
        return;
    }
    for (DataProductVec::const_iterator it = dataProducts.begin();
        it != dataProducts.end(); ++it) {

        shared_ptr<gp::Message> message;
        std::string message_data;
        
        if (((*it)->getTypeName() == "") || ((*it)->getProtocol() != "protobuf2")) {
            Log::critical("Unknown protocol (%s) or no type name.", (*it)->getProtocol().c_str());
            return;
        }

        try {
            message = _registry.createMessageByName((*it)->getTypeName());
            (*it)->populateMessage(*message);
        } catch (runtime_error &e) {
            Log::critical("Could not create or populate message (%s).",
                          (*it)->getTypeName().c_str());
            return;
        }
        
        protobufToText(*message, message_data);
        
        double moos_time = (*it)->getGravityTimestamp() / 1000000.0;
        CMOOSMsg msg(MOOS_NOTIFY, (*it)->getDataProductID(), message_data.c_str(), moos_time);
        msg.SetSource((*it)->getComponentId());
        _moosComm.Post(msg, true); // true: preserve Source Name
    }
}

} /* namespace gravity */
