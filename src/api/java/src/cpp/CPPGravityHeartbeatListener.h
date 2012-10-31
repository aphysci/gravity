
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
    virtual void MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status);
    virtual void ReceivedHeartbeat(std::string dataProductID, std::string status);
};

}
#endif /* CPPGRAVITYHEARTBEATLISTENER_H_ */

