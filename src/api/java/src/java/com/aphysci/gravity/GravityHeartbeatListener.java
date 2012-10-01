package com.aphysci.gravity;

public interface GravityHeartbeatListener {
	void MissedHeartbeat(String dataProductID, int microsecond_to_last_heartbeat, String status);
}
