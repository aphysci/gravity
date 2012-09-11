
package com.aphysci.gravity;

import com.aphysci.gravity.protobuf.GravityDataProduct;

public interface GravitySubscriber {
	public void subscriptionFilled(final GravityDataProduct dataProduct);
}
