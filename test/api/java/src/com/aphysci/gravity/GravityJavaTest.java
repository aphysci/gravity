
package com.aphysci.gravity;

import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.GravityReturnCode;
import com.aphysci.gravity.swig.Log;

public class GravityJavaTest {

	private static boolean subCalled = false;
	private static boolean reqCalled = false;
	private static boolean provCalled = false;
	private static boolean logCalled = false;
	
    public static void main(String[] argv) {
    	Log.initAndAddConsoleLogger(Log.LogLevel.DEBUG);
    	Log.initAndAddLogger(new TestLogger(), Log.LogLevel.DEBUG);
        Log.debug("in main");

        GravityNode node = new GravityNode();
        GravityReturnCode ret = node.init("TestNode");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.registerDataProduct("JavaGDP", 5678, "tcp");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        Subscriber s = new Subscriber();
        ret = node.subscribe("JavaGDP", s, "");
  
        GravityDataProduct gdp = new GravityDataProduct("JavaGDP");
        gdp.setSoftwareVersion("version 1");
        
        ret = node.publish(gdp, "Java");
        
        // wait a bit before unsubscribing to allow the message to get through
        try {
			Thread.sleep(10);
		} catch (InterruptedException e) {
		}

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
        
        // wait a bit before unsubscribing to allow the message to get through
        try {
			Thread.sleep(10);
		} catch (InterruptedException e) {
		}

        ret = node.unregisterService("JavaService");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        testAssert(subCalled);
        testAssert(reqCalled);
        testAssert(provCalled);
        testAssert(logCalled);

        Log.message("Tests OK!!");
    }
    
    private static class Subscriber implements GravitySubscriber {

		@Override
		public void subscriptionFilled(GravityDataProduct dataProduct) {
			subCalled = true;
			testAssert(dataProduct.getDataProductID().equals("JavaGDP"));
		}
    }
    
    private static class Requestor implements GravityRequestor {

		@Override
		public void requestFilled(String serviceID, String requestID,
				GravityDataProduct response) {
			reqCalled = true;
			testAssert(response.getDataProductID().equals("JavaResponse"));
		}
    }
    
    private static class ServiceProvider implements GravityServiceProvider {

		@Override
		public GravityDataProduct request(GravityDataProduct dataProduct) {
			provCalled = true;
			testAssert(dataProduct.getDataProductID().equals("JavaRequest"));
			return new GravityDataProduct("JavaResponse");
		}
    }
    
    private static class TestLogger implements Logger {
		@Override
		public void Log(int level, String messagestr) {
			System.out.println("log called (level = "+level+"): "+messagestr);
			logCalled = true;
		}
    }
    
    private static void testAssert(boolean test) {
    	if (!test) {
    		Log.fatal("Failed assertion, aborting");
    		(new Exception()).printStackTrace();
    		System.exit(1);
    	}
    }
}

