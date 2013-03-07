
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
    virtual void MissedHeartbeat(std::string componentID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds);
    virtual int64_t MissedHeartbeatJava(const std::string componentID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds);
    virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds);
    virtual int64_t ReceivedHeartbeatJava(const std::string componentID, int64_t& interval_in_microseconds);
};

}
#endif /* CPPGRAVITYHEARTBEATLISTENER_H_ */

