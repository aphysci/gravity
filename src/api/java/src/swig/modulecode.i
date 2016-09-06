/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */


// imports for gravity.java
%pragma(java) moduleimports=%{
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.WeakHashMap;
import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
import com.aphysci.gravity.GravityRequestor;
import com.aphysci.gravity.GravityServiceProvider;
import com.aphysci.gravity.GravityHeartbeatListener;
import com.aphysci.gravity.Logger;
import com.aphysci.gravity.GravitySubscriptionMonitor;
%}

/******
 *  Code for gravity.java that creates proxy classes for emulating Java interfaces to a C++ classes.
 ******/
%pragma(java) modulecode=%{
  private static Map<GravitySubscriber, CPPGravitySubscriberProxy> proxySubscriberMap =
            new WeakHashMap<GravitySubscriber, CPPGravitySubscriberProxy>();
  private static Map<GravityRequestor, CPPGravityRequestorProxy> proxyRequestorMap =
            new WeakHashMap<GravityRequestor, CPPGravityRequestorProxy>();
  private static Map<GravityServiceProvider, CPPGravityServiceProviderProxy> proxyProviderMap =
            new WeakHashMap<GravityServiceProvider, CPPGravityServiceProviderProxy>();
  private static Map<GravityHeartbeatListener, CPPGravityHeartbeatListenerProxy> proxyHeartbeatListenerMap =
            new WeakHashMap<GravityHeartbeatListener, CPPGravityHeartbeatListenerProxy>();
  private static Map<Logger, CPPGravityLoggerProxy> proxyLoggerMap =
            new WeakHashMap<Logger, CPPGravityLoggerProxy>();
  private static Map<GravitySubscriptionMonitor, CPPGravitySubscriptionMonitorProxy> proxySubscriptionMonitorMap =
            new WeakHashMap<GravitySubscriptionMonitor, CPPGravitySubscriptionMonitorProxy>();			

  private static class CPPGravitySubscriberProxy extends CPPGravitySubscriber {
    private GravitySubscriber delegate;
    public CPPGravitySubscriberProxy(GravitySubscriber i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public int subscriptionFilled(byte[] arr, int byteLength, int[] lengths, int intLength) {
      List<GravityDataProduct> dataProducts = new ArrayList<GravityDataProduct>();
      int offset = 0;
      for (int i = 0; i < intLength; i++) {
    	  byte [] singleDPArray = Arrays.copyOfRange(arr, offset, offset+lengths[i]);
    	  dataProducts.add(new GravityDataProduct(singleDPArray));
    	  offset += lengths[i];
      }
      delegate.subscriptionFilled(dataProducts);
      return 0;
    }
  }

  private static class CPPGravityRequestorProxy extends CPPGravityRequestor {
    private GravityRequestor delegate;
    public CPPGravityRequestorProxy(GravityRequestor i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public char requestFilled(String serviceID, String requestID, byte[] arr, int byteLength) {
      delegate.requestFilled(serviceID, requestID, new GravityDataProduct(arr));
      return 0;
    }
  }

  private static class CPPGravityServiceProviderProxy extends CPPGravityServiceProvider {
    private GravityServiceProvider delegate;
    public CPPGravityServiceProviderProxy(GravityServiceProvider i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public byte[] request(String serviceID, byte[] arr, int byteLength) {
      GravityDataProduct gdp = delegate.request(serviceID, new GravityDataProduct(arr));
      return gdp.serializeToArray();
    }
  }

  private static class CPPGravityHeartbeatListenerProxy extends CPPGravityHeartbeatListener {
    private GravityHeartbeatListener delegate;
    public CPPGravityHeartbeatListenerProxy(GravityHeartbeatListener i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public long MissedHeartbeatJava(String dataProductID, long microsecond_to_last_heartbeat, long[] interval_in_microseconds) {
      delegate.MissedHeartbeat(dataProductID, microsecond_to_last_heartbeat, interval_in_microseconds);
      return 0;
    }

    @SuppressWarnings("unused")
    public long ReceivedHeartbeatJava(String dataProductID, long[] interval_in_microseconds) {
      delegate.ReceivedHeartbeat(dataProductID, interval_in_microseconds);
      return 0;
    }
  }

  private static class CPPGravityLoggerProxy extends CPPGravityLogger {
    private Logger delegate;
    public CPPGravityLoggerProxy(Logger i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public void Log(int level, String messagestr) {
      delegate.Log(level, messagestr);
    }
  }
  
  private static class CPPGravitySubscriptionMonitorProxy extends CPPGravitySubscriptionMonitor {
    private GravitySubscriptionMonitor delegate;
    public CPPGravitySubscriptionMonitorProxy(GravitySubscriptionMonitor i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public void subscriptionTimeoutJava(String dataProductID, int milliSecondsSinceLast, String filter, String domain) {
      delegate.subscriptionTimeout(dataProductID, milliSecondsSinceLast, filter, domain);
    }
  }

  public static CPPGravitySubscriber makeNativeSubscriber(GravitySubscriber i) {
    if (i instanceof CPPGravitySubscriber) {
      // If it already *is* a CPPGravitySubscriber don't bother wrapping it again
      return (CPPGravitySubscriber)i;
    }
    CPPGravitySubscriberProxy proxy = proxySubscriberMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravitySubscriberProxy(i);
      proxySubscriberMap.put(i, proxy);
    }
    return proxy;
  }

  public static CPPGravityRequestor makeNativeRequestor(GravityRequestor i) {
    if (i instanceof CPPGravityRequestor) {
      // If it already *is* a CPPGravityRequestor don't bother wrapping it again
      return (CPPGravityRequestor)i;
    }
    CPPGravityRequestorProxy proxy = proxyRequestorMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravityRequestorProxy(i);
      proxyRequestorMap.put(i, proxy);
    }
    return proxy;
  }

  public static CPPGravityServiceProvider makeNativeProvider(GravityServiceProvider i) {
    if (i instanceof CPPGravityServiceProvider) {
      // If it already *is* a CPPGravityServiceProvider don't bother wrapping it again
      return (CPPGravityServiceProvider)i;
    }
    CPPGravityServiceProviderProxy proxy = proxyProviderMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravityServiceProviderProxy(i);
      proxyProviderMap.put(i, proxy);
    }
    return proxy;
  }

  public static CPPGravityHeartbeatListener makeNativeHeartbeatListener(GravityHeartbeatListener i) {
    if (i instanceof CPPGravityHeartbeatListener) {
      // If it already *is* a CPPGravityHeartbeatListener don't bother wrapping it again
      return (CPPGravityHeartbeatListener)i;
    }
    CPPGravityHeartbeatListenerProxy proxy = proxyHeartbeatListenerMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravityHeartbeatListenerProxy(i);
      proxyHeartbeatListenerMap.put(i, proxy);
    }
    return proxy;
  }

  public static CPPGravityLogger makeNativeLogger(Logger i) {
    if (i instanceof CPPGravityLogger) {
      // If it already *is* a CPPGravityLogger don't bother wrapping it again
      return (CPPGravityLogger)i;
    }
    CPPGravityLoggerProxy proxy = proxyLoggerMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravityLoggerProxy(i);
      proxyLoggerMap.put(i, proxy);
    }
    return proxy;
  }
  
  public static CPPGravitySubscriptionMonitor makeNativeSubscriptionMonitor(GravitySubscriptionMonitor i) {
    if (i instanceof CPPGravitySubscriptionMonitor) {
      // If it already *is* a CPPGravitySubscriptionMonitor don't bother wrapping it again
      return (CPPGravitySubscriptionMonitor)i;
    }
    CPPGravitySubscriptionMonitorProxy proxy = proxySubscriptionMonitorMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravitySubscriptionMonitorProxy(i);
      proxySubscriptionMonitorMap.put(i, proxy);
    }
    return proxy;
  }
  
%}
