
package com.aphysci.gravity;

import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.GravityReturnCode;

public class GravityJavaTest {

    public static void main(String[] argv) throws InterruptedException {
        System.out.println("in main");

        GravityNode node = new GravityNode();
        GravityReturnCode ret = node.init();
        assert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.registerDataProduct("JavaGDP", 5678, "tcp");
        assert(ret == GravityReturnCode.SUCCESS);
        
        Subscriber s = new Subscriber();
        ret = node.subscribe("JavaGDP", s, "");
  
        GravityDataProduct gdp = new GravityDataProduct("JavaGDP");
        gdp.setSoftwareVersion("version 1");
        
        ret = node.publish(gdp, "Java");
        
        ret = node.unsubscribe("JavaGDP", s, "");
        assert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.publish(gdp, "Java");
        
        ret = node.unregisterDataProduct("JavaGDP");
        assert(ret == GravityReturnCode.SUCCESS);

        ret = node.unregisterDataProduct("JavaGDP");
        assert(ret == GravityReturnCode.REGISTRATION_CONFLICT);
        
        GravityServiceProvider gsp = new ServiceProvider();
        ret = node.registerService("JavaService", (short)8888, "tcp", gsp);
        assert(ret == GravityReturnCode.SUCCESS);
        
        GravityRequestor gr = new Requestor();
        GravityDataProduct request = new GravityDataProduct("Request");
        ret = node.request("JavaService", request, gr);
        assert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.unregisterService("JavaService");
        assert(ret == GravityReturnCode.SUCCESS);
//        Thread.sleep(10000);
    }
    
    private static class Subscriber implements GravitySubscriber {

		@Override
		public void subscriptionFilled(GravityDataProduct dataProduct) {
			System.out.println("made it to java callback, gdp = "+dataProduct);
			if (dataProduct != null) 
				System.out.println("dataProduct id = " + dataProduct.getDataProductID());
		}
    }
    
    private static class Requestor implements GravityRequestor {

		@Override
		public void requestFilled(String serviceID, String requestID,
				GravityDataProduct response) {
			System.out.println("Made it Java Request filled for serviceID = "+serviceID);
		}
    }
    
    private static class ServiceProvider implements GravityServiceProvider {

		@Override
		public GravityDataProduct request(GravityDataProduct dataProduct) {
			System.out.println("Made it to Java Received service request with dataProduct id = "+dataProduct.getDataProductID());
			return new GravityDataProduct("JavaResponse");
		}
    	
    }
}

