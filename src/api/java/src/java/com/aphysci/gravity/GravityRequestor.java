package com.aphysci.gravity;

public interface GravityRequestor {
	void requestFilled(String serviceID, String requestID, GravityDataProduct response);
}
