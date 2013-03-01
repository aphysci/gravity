/*
 * GravitySubscriber.h
 *
 *  Created on: Sept. 19, 2012
 *      Author: Tim Ludwinski
 */

#ifndef GRAVITYHEARTBEATLISTENER_H_
#define GRAVITYHEARTBEATLISTENER_H_

#include <string>

namespace gravity
{

/**
 * Interface specification for an object that will respond to connection outages of gravity products.
 */
class GravityHeartbeatListener
{
public:
	/**
	 * Called when another component's heartbeat is off by a certain amount.
	 * \param dataProductID component id for the component whose heartbeat was missed
	 * \microsecond_to_last_heartbeat number of microseconds since the last heartbeat was received, or -1 if
	 * a heart beat was never received.
	 * \param status string that indicates the current status.  Currently always "Missed".
	 */
	virtual void MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status) = 0;

	/**
	 * Called when another component's heartbeat is received
     * \param dataProductID component id for the component whose heartbeat was received
     * \param status string that indicates the current status.  Currently always "Received".
	 */
	virtual void ReceivedHeartbeat(std::string dataProductID, std::string status) = 0;

    /**
     * Default destructor
     */
	virtual ~GravityHeartbeatListener() { }
};

} /* namespace gravity */
#endif //GRAVITYHEARTBEATLISTENER_H_
