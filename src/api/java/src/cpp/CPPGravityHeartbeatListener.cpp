

#include "CPPGravityHeartbeatListener.h"
#include <iostream>

using namespace gravity;

CPPGravityHeartbeatListener::~CPPGravityHeartbeatListener()
{}

void CPPGravityHeartbeatListener::MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status)
{
}

void CPPGravityHeartbeatListener::ReceivedHeartbeat(std::string dataProductID, std::string status)
{
}

