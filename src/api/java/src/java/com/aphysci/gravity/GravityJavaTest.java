
package com.aphysci.gravity;

import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.GravityReturnCode;

public class GravityJavaTest {

    public static void main(String[] argv) {
        System.out.println("in main");

        GravityNode node = new GravityNode();
        GravityReturnCode ret = node.init();
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.registerDataProduct("JavaGDP", 5678, "tcp");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        Subscriber s = new Subscriber();
        ret = node.subscribe("JavaGDP", s, "");
  
        GravityDataProduct gdp = new GravityDataProduct("JavaGDP");
        gdp.setSoftwareVersion("version 1");
        
        ret = node.publish(gdp, "Java");
        
        ret = node.unsubscribe("JavaGDP", s, "");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.publish(gdp, "Java");
        
        ret = node.unregisterDataProduct("JavaGDP");
        testAssert(ret == GravityReturnCode.SUCCESS);

        ret = node.unregisterDataProduct("JavaGDP");
        testAssert(ret == GravityReturnCode.REGISTRATION_CONFLICT);
        
        GravityServiceProvider gsp = new ServiceProvider();
        ret = node.registerService("JavaService", (short)8888, "tcp", gsp);
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        GravityRequestor gr = new Requestor();
        GravityDataProduct request = new GravityDataProduct("JavaRequest");
        ret = node.request("JavaService", request, gr);
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.unregisterService("JavaService");
        testAssert(ret == GravityReturnCode.SUCCESS);

        System.out.println("Tests OK!!");
    }
    
    private static class Subscriber implements GravitySubscriber {

		@Override
		public void subscriptionFilled(GravityDataProduct dataProduct) {
			testAssert(dataProduct.getDataProductID().equals("JavaGDP"));
		}
    }
    
    private static class Requestor implements GravityRequestor {

		@Override
		public void requestFilled(String serviceID, String requestID,
				GravityDataProduct response) {
			testAssert(response.getDataProductID().equals("JavaResponse"));
		}
    }
    
    private static class ServiceProvider implements GravityServiceProvider {

		@Override
		public GravityDataProduct request(GravityDataProduct dataProduct) {
			testAssert(dataProduct.getDataProductID().equals("JavaRequest"));
			return new GravityDataProduct("JavaResponse");
		}
    }
    
    private static void testAssert(boolean test) {
    	if (!test) {
    		System.err.println("Failed assertion, aborting");
    		(new Exception()).printStackTrace();
    		System.exit(1);
    	}
    }
}

