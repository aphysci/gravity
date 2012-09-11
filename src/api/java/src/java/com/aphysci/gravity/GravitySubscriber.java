
package com.aphysci.gravity;

import gravity.GravityDataProduct;

public interface GravitySubscriber {
	public void subscriptionFilled(final GravityDataProduct dataProduct);
}
