
package com.aphysci.gravity;

import java.util.List;

import com.aphysci.gravity.protobuf.JavaTestContainer.JavaTestPB;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.GravityReturnCode;
import com.aphysci.gravity.swig.Log;

public class GravityJavaTest {

	private static int subCount = 0;
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
        
        ret = node.registerDataProduct("JavaGDP", "tcp");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        Subscriber s = new Subscriber();
        ret = node.subscribe("JavaGDP", s, "");
  
        GravityDataProduct gdp = new GravityDataProduct("JavaGDP");
        gdp.setSoftwareVersion("version 1");
        JavaTestPB.Builder builder = JavaTestPB.newBuilder();
        int count = 0;
        builder.setCount(count++);
        builder.setMessage("Hello Java World");
        gdp.setData(builder);
        
        ret = node.publish(gdp, "Java");
        testAssert(ret == GravityReturnCode.SUCCESS);

        builder.setCount(count++);
        gdp.setData(builder);
        ret = node.publish(gdp, "Java");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        builder.setCount(count++);
        gdp.setData(builder);
        ret = node.publish(gdp, "Java");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        builder.setCount(count++);
        gdp.setData(builder);
        ret = node.publish(gdp, "Java");
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        // wait a bit before unsubscribing to allow the messages to get through
        try {
			Thread.sleep(2000);
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
        ret = node.registerService("JavaService", "tcp", gsp);
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
        
        ret = node.registerService("SyncJavaService", "tcp", new SyncServiceProvider());
        testAssert(ret == GravityReturnCode.SUCCESS);
        
        GravityDataProduct syncRequest = new GravityDataProduct("SyncJavaRequest");
        GravityDataProduct syncResponse = node.request("SyncJavaService", syncRequest);
        testAssert(syncResponse.getDataProductID().equals("SyncJavaResponse"));
        
        testAssert(subCount == 4);
        testAssert(reqCalled);
        testAssert(provCalled);
        testAssert(logCalled);

        Log.message("Tests OK!!");
    }
    
    private static class Subscriber implements GravitySubscriber {

		@Override
		public void subscriptionFilled(final List<GravityDataProduct> dataProducts) {
			for (GravityDataProduct gdp : dataProducts) {
				testAssert(gdp.getDataProductID().equals("JavaGDP"));
				testAssert(gdp.getSoftwareVersion().equals("version 1"));

				JavaTestPB.Builder builder = JavaTestPB.newBuilder();
				gdp.populateMessage(builder);
				JavaTestPB pb = builder.build();
				testAssert(pb.getCount() == subCount);
				testAssert(pb.getMessage().equals("Hello Java World"));
				subCount++;
				
				Log.debug("Got GDP");
			}
			// sleep to give messages a chance to queue up.
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
			}
		}
    }
    
    private static class Requestor implements GravityRequestor {

		@Override
		public void requestFilled(String serviceID, String requestID,
				GravityDataProduct response) {
			reqCalled = true;
			Log.debug("Got Request");
			testAssert(response.getDataProductID().equals("JavaResponse"));
		}
    }
    
    private static class ServiceProvider implements GravityServiceProvider {

		@Override
		public GravityDataProduct request(String serviceID, GravityDataProduct dataProduct) {
			Log.debug("Request Made: " + serviceID);
			provCalled = true;
			testAssert(dataProduct.getDataProductID().equals("JavaRequest"));
			return new GravityDataProduct("JavaResponse");
		}
    }

    private static class SyncServiceProvider implements GravityServiceProvider {

		@Override
		public GravityDataProduct request(String serviceID, GravityDataProduct dataProduct) {
			Log.debug("Sync Request Made: " + serviceID);
			testAssert(dataProduct.getDataProductID().equals("SyncJavaRequest"));
			return new GravityDataProduct("SyncJavaResponse");
		}
    }

    private static class TestLogger implements Logger {
		@Override
		public void Log(int level, String messagestr) {
			logCalled = Log.LogLevel.swigToEnum(level) == Log.LogLevel.DEBUG;
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

