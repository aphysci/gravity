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
	 */
	virtual void MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status) = 0;

    /**
     * Default destructor
     */
	virtual ~GravityHeartbeatListener() { }
};

} /* namespace gravity */
#endif //GRAVITYHEARTBEATLISTENER_H_
