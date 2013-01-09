package com.aphysci.gravity;

public interface GravityServiceProvider {
	GravityDataProduct request(String serviceID, GravityDataProduct dataProduct);
}
