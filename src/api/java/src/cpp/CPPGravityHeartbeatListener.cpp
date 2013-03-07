

#include "CPPGravityHeartbeatListener.h"
#include <iostream>

using namespace gravity;
using namespace std;

CPPGravityHeartbeatListener::~CPPGravityHeartbeatListener()
{}

void CPPGravityHeartbeatListener::MissedHeartbeat(std::string componentID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds)
{
    MissedHeartbeatJava(componentID, microsecond_to_last_heartbeat, interval_in_microseconds);
}

int64_t CPPGravityHeartbeatListener::MissedHeartbeatJava(const std::string componentID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds)
{
    cout << "in cpp HB listener missed heartbeat" << endl;
}

void CPPGravityHeartbeatListener::ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds)
{
    ReceivedHeartbeatJava(componentID, interval_in_microseconds);
}

int64_t CPPGravityHeartbeatListener::ReceivedHeartbeatJava(const std::string componentID, int64_t& interval_in_microseconds)
{
    cout << "in cpp HB listener received heartbeat" << endl;
}

