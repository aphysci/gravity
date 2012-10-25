
package com.aphysci.gravity;

import java.util.List;

public interface GravitySubscriber {
	public void subscriptionFilled(final List<GravityDataProduct> dataProducts);
}
