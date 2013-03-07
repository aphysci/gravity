package com.aphysci.gravity;

public interface GravityHeartbeatListener {
	/**
	 * 
	 * @param componentID id of the component whose heart beat has not been received
	 * @param microsecond_to_last_heartbeat microseconds since the last heart beat was received
	 * from the given component, or -1 if a heart beat was never received.    
	 * @param interval_in_microseconds an array of length 1 that contains the current interval at which
	 * a heart beat is expected.  Updates to this value will change the interval used in subsequent iterations.
	 */
	void MissedHeartbeat(String componentID, long microsecond_to_last_heartbeat, long[] interval_in_microseconds);
	
	/**
	 * 
	 * @param componentID id of the component whose heart beat has been received
	 * @param interval_in_microseconds an array of length 1 that contains the current interval at which
	 * a heart beat is expected.  Updates to this value will change the interval used in subsequent iterations.
	 */
	void ReceivedHeartbeat(String componentID, long[] interval_in_microseconds);
}
