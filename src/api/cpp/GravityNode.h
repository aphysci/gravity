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

/*
 * GravityNode.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYNODE_H_
#define GRAVITYNODE_H_

#include "GravitySubscriber.h"
#include "GravityRequestor.h"
#include "GravityHeartbeatListener.h"
#include "GravitySemaphore.h"
#include "GravityServiceProvider.h"
#include "GravitySubscriptionMonitor.h"
#include "Utility.h"
#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include <thread>
#include <list>

//This is defined in Windows for NetBIOS in nb30.h
#ifdef DUPLICATE
#undef DUPLICATE
#endif

// This is defined in Windows, and causes ezOptionParser to not compile
#ifdef IN
#undef IN
#endif

namespace gravity
{

/**
 * Namespace to hold Gravity Return Codes.
 */
namespace GravityReturnCodes
{
    /**
     * Return codes used on a Gravity system.
     */
    GRAVITY_API enum Codes
    {
        /*
         * If you add to this list, also add to GravityNode::getCodeString()
         */
        SUCCESS = 0, ///< The request was successful
        FAILURE = -1, ///< The request failed
        NO_SERVICE_DIRECTORY = -2, ///< Could not find Service Directory
        REQUEST_TIMEOUT = -3, ///< The request timed out while waiting for a response.
        DUPLICATE = -4, ///< Attempting to register an existing data product ID
        REGISTRATION_CONFLICT = -5, ///< Attempting to unregister from a service or dataProductID that is not currently registered.
        NOT_REGISTERED = -6, ///< TBD
        NO_SUCH_SERVICE = -7, ///< Made a bad request (Ex: used a non-existent domain or submitted a request before service was available)
        LINK_ERROR = -8, ///< Error serializing or deserializing a Gravity Data Product 
        INTERRUPTED = -9, ///< Received an interruption.
        NO_SERVICE_PROVIDER = -10, ///< No service provider found.
        NO_PORTS_AVAILABLE = -11, ///< No ports available
		INVALID_PARAMETER = -12, ///< Invalid parameter. (Ex: entered a negative number for time)
		NOT_INITIALIZED = -13  ///< The GravityNode has not successfully completed initialization yet (i.e. init has not been called or did not succeed).
    };
}
typedef GravityReturnCodes::Codes GravityReturnCode;

/**
 * Namespace to hold Gravity Transport Types.
 */
namespace GravityTransportTypes
{
   /**
    * Network transport protocols available on a Gravity system. 
    */
   enum Types
   {
      TCP = 0, ///< Transmission Control Protocol
      INPROC = 1, ///< In-process (Inter-thread) Communication 
      PGM = 2, ///< Pragmatic General Multicast Protocol
      EPGM= 3, ///< Encapsulated PGM
#ifndef WIN32
      IPC = 4 ///< Inter-Process Communication
#endif
   };
}
typedef GravityTransportTypes::Types GravityTransportType;

typedef struct SocketWithLock
{
	void *socket = nullptr;
	Semaphore lock;
} SocketWithLock;

/*
typedef struct GravityINIConfig
{
	void* context;
    std::string domain;
    int port;
    int timeout;
    GravityNode *gravityNode;
    std::string componentId;
} GravityINIConfig;
*/

class GravityConfigParser;
class FutureResponse;

/**
 * A component that provides a simple interface point to a Gravity-enabled application
 */
class GravityNode
{
private:
  
  /**
   * Domain change listener for a GravityNode.
   */
	class GravityNodeDomainListener
	{
	public:
    /**
     * Return the current Gravity domain.
     */
		std::string getDomainUrl();

    /**
     * Listen for and implement a gravity domain change for the given GravityNode.
     * Continues until the it is interruption or receives a kill command. 
     */
		void start();

		GravityNodeDomainListener(void *context);

		virtual ~GravityNodeDomainListener();

	private:
		int sock;
		bool running;
		bool ready;
		void* context;
		std::string domain;
		std::string compId;
		int port;
		int timeout;
		GravityNode* gravityNode;
		void* gravityNodeSocket;

		Semaphore lock;

		void readDomainListenerParameters();
	};

    typedef struct NetworkNode
    {
        std::string ipAddress;
        unsigned short port;
        std::string transport;
        void* socket;
    } NetworkNode;

    typedef struct SubscriptionDetails
    {
        std::string domain;
        std::string dataProductID;
        std::string filter;
		bool receiveLastCachedValue;
		bool isRelay;
        const GravitySubscriber* subscriber;
    } SubscriptionDetails;

    static const int NETWORK_TIMEOUT = 3000; // msec
    static const int NETWORK_RETRIES = 3; // attempts to connect
    bool metricsEnabled;
	bool initialized;
	bool logInitialized;
	bool listenerEnabled;
	bool heartbeatStarted;

	bool defaultCacheLastSentDataprodut;
	bool defaultReceiveLastSentDataproduct;
	
  std::thread subscriptionManagerThread;

    void* context = nullptr;
    SocketWithLock subscriptionManagerSWL;
    SocketWithLock subscriptionManagerConfigSWL;
    SocketWithLock publishManagerRequestSWL;
    SocketWithLock publishManagerPublishSWL;
    SocketWithLock serviceManagerSWL;
	SocketWithLock serviceManagerConfigSWL;
    SocketWithLock requestManagerSWL;
	SocketWithLock requestManagerRepSWL;
	SocketWithLock domainListenerSWL;
	SocketWithLock domainRecvSWL;
    void* metricsManagerSocket = nullptr; // only used in init, no lock needed
    void* hbSocket; // Inproc socket for adding requests to heartbeat listener thread.

	std::string listenForBroadcastURL(std::string domain, int port, int timeout);
   
    GravityReturnCode sendRequestToServiceDirectory(const GravityDataProduct& request, GravityDataProduct& response);
    GravityReturnCode sendRequestsToServiceProvider(std::string url, const GravityDataProduct& request, GravityDataProduct& response,
    		int timeout_in_milliseconds, int retries);
    GravityReturnCode sendRequestToServiceProvider(std::string url, const GravityDataProduct& request, GravityDataProduct& response,
    		int timeout_in_milliseconds);

    NetworkNode serviceDirectoryNode;
    Semaphore serviceDirectoryLock;
    std::map<std::string,std::string> publishMap;
    std::map<std::string,std::string> serviceMap; ///< Maps serviceID to url
    std::list<SubscriptionDetails> subscriptionList;
	std::map<std::string,uint64_t> urlInstanceMap;
    std::string myDomain;
    std::string componentID;
	std::map<std::string, uint32_t> dataRegistrationTimeMap; // Maps data product id to registration time
	GravityConfigParser* parser;

	GravityReturnCode ServiceDirectoryServiceLookup(std::string serviceOrDPID, std::string &url, std::string &domain, uint32_t &regTime);
	GravityReturnCode ServiceDirectoryDataProductLookup(std::string serviceOrDPID, std::vector<gravity::PublisherInfoPB> &urls, std::string &domain);
    GravityReturnCode ServiceDirectoryReregister(std::string componentId, std::string url);

	void updateServiceDirectoryUrl(std::string serviceDirectoryUrl);

    // Separate actual functionality of sub/unsub methods so that they can be locked correctly
    GravityReturnCode subscribeInternal(std::string dataProductID, const GravitySubscriber& subscriber,
                                            std::string filter, std::string domain, bool receiveLastCachedValue = true);
    GravityReturnCode unsubscribeInternal(std::string dataProductID, const GravitySubscriber& subscriber,
                                                std::string filter, std::string domain);

    GravityReturnCode request(std::string connectionURL, std::string serviceID, const GravityDataProduct& dataProduct,
		const GravityRequestor& requestor, uint32_t regTime, std::string requestID = "", int timeout_milliseconds = -1);

    GRAVITY_API GravityReturnCode registerDataProductInternal(std::string dataProductID, GravityTransportType transportType,
    		                                                  bool cacheLastValue, bool isRelay, bool localOnly);

	static void* startGravityDomainListener(void* context);
	
	void configureNodeDomainListener(std::string domain);
	
	void configureServiceManager();
	void configureSubscriptionManager();

	std::string getDomainUrl(int timeout);

public:
    /**
     * Default Constructor
     */
    GRAVITY_API GravityNode();

	/**
	* Constructor that also initializes
	* \param componentID ID of the component to initialize
	*/
	GRAVITY_API GravityNode(std::string componentID);

    /**
     * Default Destructor
     */
    GRAVITY_API virtual ~GravityNode();

	/**
     * Initialize the Gravity infrastructure.
	   * Reads the ComponentID from the Gravity.ini file.
     * \return GravityReturnCode code to identify any errors that occur during initialization
     */
    GRAVITY_API GravityReturnCode init();

    /**
     * Initialize the Gravity infrastructure.
     * \copydetails GravityNode(std::string)
     * \return GravityReturnCode code to identify any errors that occur during initialization
     */
    GRAVITY_API GravityReturnCode init(std::string componentID);

    /**
     * Wait for the GravityNode to exit.
     */
    GRAVITY_API void waitForExit();

    /**
     * Setup a subscription to a data product through the Gravity Service Directory.
     * \param dataProductID string ID of the data product of interest
     * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
     * \return success flag
     */
	GRAVITY_API GravityReturnCode subscribe(std::string dataProductID, const GravitySubscriber& subscriber);

    /**
     * \copybrief subscribe(std::string,const GravitySubscriber&)
     * \param filter text filter to apply to subscription
     * \copydetails subscribe(std::string,const GravitySubscriber&)
     */
	GRAVITY_API GravityReturnCode subscribe(std::string dataProductID, const GravitySubscriber& subscriber, std::string filter);

    /**
     * \copybrief subscribe(std::string,const GravitySubscriber&,std::string)
     * \param domain domain of the network components
     * \copydetails subscribe(std::string,const GravitySubscriber&,std::string)
     */
	GRAVITY_API GravityReturnCode subscribe(std::string dataProductID, const GravitySubscriber& subscriber, std::string filter, std::string domain);

    /**
     * \copybrief subscribe(std::string,const GravitySubscriber&,std::string,std::string)
     * \param receiveLastCachedValue
     * \copydetails subscribe(std::string,const GravitySubscriber&,std::string,std::string)
     */
	GRAVITY_API GravityReturnCode subscribe(std::string dataProductID, const GravitySubscriber& subscriber, std::string filter, std::string domain, bool receiveLastCachedValue);

    /**
     * Un-subscribe from a data product
     * \param dataProductID ID of data product for which subscription is to be removed
     * \param subscriber the subscriber that will be removed from the notification list for this subscription
     * \param filter text filter associated with the subscription to cancel
     * \param domain domain of the network components
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unsubscribe(std::string dataProductID, const GravitySubscriber& subscriber, 
												std::string filter="", std::string domain = "");

    /**
     * Publish a data product to the Gravity Service Directory.
     * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
     * \param filterText text filter associated with the publish
     * \param timestamp time dataProduct was created
     * \return success flag
     */
    GRAVITY_API GravityReturnCode publish(const GravityDataProduct& dataProduct, std::string filterText = "", uint64_t timestamp = 0);

    /**
     * Determine whether there currently any subscribers for this DataProduct
     * \param dataProduct GravityDataProduct for which to check for the presence of subscribers
     * \return false if there are no subscribers, otherwise true
     **/
    GRAVITY_API bool querySubscribers(std::string dataProductID);

    /**
     * Make an asynchronous request against a service provider through the Gravity Service Directory
     * \param serviceID The registered service ID of a service provider
     * \param request data product representation of the request
     * \param requestor object implementing the GravityRequestor interface that will be notified of the response
     * \param requestID identifier for this request
     * \param timeout_milliseconds Timeout in Milliseconds (-1 for no timeout)
     * \param domain domain of the network components
     * \return success flag
     */
    GRAVITY_API GravityReturnCode request(std::string serviceID, const GravityDataProduct& request,
										const GravityRequestor& requestor, std::string requestID = "", 
										int timeout_milliseconds = -1, std::string domain = "");	

    /**
     * Make a synchronous request against a service provider
     * \param serviceID The registered service ID of a service provider
     * \param request data product representation of the request
     * \param timeout_milliseconds Timeout in Milliseconds (-1 for no timeout)
     * \param domain domain of the network components
     * \return shared_ptr<GravityDataProduct> pointer to the data product representation of the response. NULL upon failure.
     */
    GRAVITY_API std::shared_ptr<GravityDataProduct> request(std::string serviceID, const GravityDataProduct& request,
										int timeout_milliseconds = -1, std::string domain = "");

    /**
     * Starts a heart beat for this GravityNode.
     * \param interval_in_microseconds interval that heart beats are sent.  Typed as a signed 64 bit integer to
     * make passing to Java via Swig cleaner.
     * \return success flag
     */
    GRAVITY_API GravityReturnCode startHeartbeat(int64_t interval_in_microseconds);

	/**
     * Stops the heart beat for this GravityNode.
     * \return success flag
	 */
	GRAVITY_API GravityReturnCode stopHeartbeat();

    /**
     * @name Gravity.ini parsing functions
     * @{
     *  Functions to extract parameters from the Gravity.ini
     *  \param key lookup key
     *  \param default_value default value to return if Gravity.ini does not contain the key
     */
    GRAVITY_API std::string getStringParam(std::string key, std::string default_value = "");
    GRAVITY_API int getIntParam(std::string key, int default_value = -1);
    GRAVITY_API double getFloatParam(std::string key, double default_value = 0.0);
    GRAVITY_API bool getBoolParam(std::string key, bool default_value = false);
    /** @} */ //Gravity.ini parsing functions

    /**
     * Get the ID of this gravity node (given in the init function).
     */
    GRAVITY_API std::string getComponentID();

    /**
     * @name Registration functions
     * @{
     *  These presumably must only be accessed by one thread at a time (this is true for registerHeartbeatListener,
     *   registerDataProduct doesn't synchronize access to publishMap either [besides that it's ok]).
     */

    /**
     * Register a data product with the Gravity, and optionally, the Directory Service, making it available to the
     * rest of the Gravity-enabled system.
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param transportType transport type (e.g. 'tcp', 'ipc')
     * \return success flag
     */
	GRAVITY_API GravityReturnCode registerDataProduct(std::string dataProductID, GravityTransportType transportType);
	 /**
     * Register a data product with the Gravity, and optionally, the Directory Service, making it available to the
     * rest of the Gravity-enabled system.
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param transportType transport type (e.g. 'tcp', 'ipc')
	 * \param cacheLastValue flag used to signify whether or not GravityNode will cache the last sent value for a published dataproduct
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerDataProduct(std::string dataProductID, GravityTransportType transportType, bool cacheLastValue);

    /**
     * Un-register a data product, resulting in its removal from the Gravity Service Directory
     * \param dataProductID string ID used to uniquely identify this published data product
     */
    GRAVITY_API GravityReturnCode unregisterDataProduct(std::string dataProductID);
    /**
     * Register as a service provider with Gravity, and optionally, the Service Directory
     * \param serviceID Unique ID with which to register this service
     * \param transportType transport type for requests (e.g. 'tcp', 'ipc')
     * \param server object implementing the GravityServiceProvider interface that will be notified of requests
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerService(std::string serviceID, GravityTransportType transportType,
            const GravityServiceProvider& server);
    /**
     * Unregister as a service provider with the Gravity Service Directory
     * \param serviceID Unique ID with which the service was originally registered
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unregisterService(std::string serviceID);

    /**
     * Registers a callback to be called when we don't get a heartbeat from another component.
     * \param componentID Look for heart beats from this component
     * \param interval_in_microseconds interval that heart beats are expected.  Typed as a signed 64 bit integer to
     * make passing to Java via Swig cleaner.
     * \param listener instance of a GravityHeartbeatListener that will be notified when heart beats are received or missed.
	 * \param domain name of domain for the componentID
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerHeartbeatListener(std::string componentID, int64_t interval_in_microseconds, 
			const GravityHeartbeatListener& listener, std::string domain = "");

	  /**
     * Unregisters a callback for when we get a heartbeat from another component.
	   * \param componentID name of component we are currently registered to
	   * \param domain name of domain for the componentID
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unregisterHeartbeatListener(std::string componentID, std::string domain = "");

    /**
     * Register a Relay that will act as a pass-through for the given dataProductID.  It will be a publisher and subscriber
     * for the given dataProductID, but other components will only subscribe to this data if they are on the same host (localOnly == true), or
     * if it is acting as a global relay (localOnly == false).  The Gravity infrastructure automatically handles which components should
     * receive relayed or non-relayed data.
     *
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
     * \param localOnly specifies whether the registered relay will provide data to their own host only, or components on any host looking for this dataProductID
     * \param transportType transport type (e.g. 'tcp', 'ipc')
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerRelay(std::string dataProductID, const GravitySubscriber& subscriber, bool localOnly, GravityTransportType transportType);

    /**
     * Register a Relay that will act as a pass-through for the given dataProductID.  It will be a publisher and subscriber
     * for the given dataProductID, but other components will only subscribe to this data if they are on the same host (localOnly == true), or
     * if it is acting as a global relay (localOnly == false).  The Gravity infrastructure automatically handles which components should
     * receive relayed or non-relayed data.
     *
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
     * \param localOnly specifies whether the registered relay will provide data to their own host only, or components on any host looking for this dataProductID
     * \param transportType transport type (e.g. 'tcp', 'ipc')
     * \param cacheLastValue flag used to signify whether or not GravityNode will cache the last sent value for a published dataproduct
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerRelay(std::string dataProductID, const GravitySubscriber& subscriber, bool localOnly, GravityTransportType transportType, bool cacheLastValue);

    /**
     * Unregister a relay for the given dataProductID.  Handles unregistering as a publisher and subscriber.
     *
     * \param dataProductID ID of data product for which the relay is to be removed as a publisher and subscriber
     * \param subscriber the subscriber that will be removed from the notification list for this subscription
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unregisterRelay(std::string dataProductID, const GravitySubscriber& subscriber);
    /** @} */ //Registration functions

    /**
     * Returns a string representation of the provided error code.
     */
    GRAVITY_API std::string getCodeString(GravityReturnCode code);

	/**
	* Utility method to get the host machine's IP address
	**/
	GRAVITY_API std::string getIP();  

	/**
	 * Returns the domain with which this node is associated
	 **/
	GRAVITY_API std::string getDomain();

	/**
	 * Creates and returns a FutureReponse pointer for delayed response to requests
	 */
	GRAVITY_API std::shared_ptr<FutureResponse> createFutureResponse();

	/**
	 * Send a FutureResponse
   * \return success flag
	 */
	GRAVITY_API GravityReturnCode sendFutureResponse(const FutureResponse& futureResponse);

  /**
   * Setup a GravitySubscriptionMonitor to receive subscription timeout information through the Gravity Service Directory.
   * \param dataProductID the ID of the data product to monitor 
   * \param milliSecondTimeout the time elapsed since receiving a new data product that qualifies as a subscription timeout 
   * \return success flag
   */
	GRAVITY_API GravityReturnCode setSubscriptionTimeoutMonitor(std::string dataProductID, const GravitySubscriptionMonitor& monitor, 
			int milliSecondTimeout, std::string filter="", std::string domain="");

  /**
   * Remove the given dataProductID from the given GravitySubscriptionMonitor. 
   * \return success  flag
   */
	GRAVITY_API GravityReturnCode clearSubscriptionTimeoutMonitor(std::string dataProductID, const GravitySubscriptionMonitor& monitor, 
			std::string filter="", std::string domain="");

};

} /* namespace gravity */
#endif /* GRAVITYNODE_H_ */
