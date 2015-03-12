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
#include "Utility.h"
#include <pthread.h>
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
 * Enumerated Type for Gravity Return Codes
 */
namespace GravityReturnCodes
{
    enum Codes
    {
        /*
         * If you add to this list, also add to GravityNode::getCodeString()
         */
        SUCCESS = 0,
        FAILURE = -1,
        NO_SERVICE_DIRECTORY = -2,
        REQUEST_TIMEOUT = -3,
        DUPLICATE = -4,
        REGISTRATION_CONFLICT = -5,
        NOT_REGISTERED = -6,
        NO_SUCH_SERVICE = -7,
        LINK_ERROR = -8,
        INTERRUPTED = -9,
        NO_SERVICE_PROVIDER = -10,
        NO_PORTS_AVAILABLE = -11
    };
}
typedef GravityReturnCodes::Codes GravityReturnCode;


namespace GravityTransportTypes
{
   enum Types
   {
      TCP = 0,
      INPROC = 1,
      PGM = 2,
      EPGM= 3,
#ifndef WIN32
      IPC = 4
#endif
   };
}
typedef GravityTransportTypes::Types GravityTransportType;

typedef struct SocketWithLock
{
	void *socket;
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

/**
 * The GravityNode is a component that provides a simple interface point to a Gravity-enabled application
 */
class GravityNode
{
private:
	class GravityNodeDomainListener
	{
	public:
		std::string getDomainUrl();
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
        const GravitySubscriber* subscriber;
    } SubscriptionDetails;

    static const int NETWORK_TIMEOUT = 3000; // msec
    static const int NETWORK_RETRIES = 3; // attempts to connect
    bool metricsEnabled;
	bool initialized;
	bool logInitialized;
	bool listenerEnabled;

    pthread_t subscriptionManagerThread;
    pthread_t publishManagerThread;
    pthread_t requestManagerThread;
    pthread_t serviceManagerThread;
    pthread_t metricsManagerThread;
	pthread_t domainListenerThread;

    void* context;
    SocketWithLock subscriptionManagerSWL;
    SocketWithLock publishManagerRequestSWL;
    SocketWithLock publishManagerPublishSWL;
    SocketWithLock serviceManagerSWL;
	SocketWithLock serviceManagerConfigSWL;
    SocketWithLock requestManagerSWL;
	SocketWithLock domainListenerSWL;
	SocketWithLock domainRecvSWL;
    void* metricsManagerSocket; // only used in init, no lock needed
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

    std::string myDomain;
    std::string componentID;
	GravityConfigParser* parser;

	GravityReturnCode ServiceDirectoryServiceLookup(std::string serviceOrDPID, std::string &url, std::string &domain);
	GravityReturnCode ServiceDirectoryDataProductLookup(std::string serviceOrDPID, std::vector<std::string> &urls, std::string &domain);
    GravityReturnCode ServiceDirectoryReregister(std::string componentId);

    // Separate actual functionality of sub/unsub methods so that they can be locked correctly
    GravityReturnCode subscribeInternal(std::string dataProductID, const GravitySubscriber& subscriber,
                                            std::string filter, std::string domain);
    GravityReturnCode unsubscribeInternal(std::string dataProductID, const GravitySubscriber& subscriber,
                                                std::string filter, std::string domain);

    GravityReturnCode subscribe(std::string connectionURL, std::string dataProductID,
            const GravitySubscriber& subscriber, std::string filter = "", std::string domain = "");

    GravityReturnCode request(std::string connectionURL, std::string serviceID, const GravityDataProduct& dataProduct,
            const GravityRequestor& requestor, std::string requestID = "", int timeout_milliseconds = -1);

	static void* startGravityDomainListener(void* context);
	
	void configureNodeDomainListener(std::string domain);
	
	void configureServiceManager();

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
     * \param componentID ID of the component to initialize
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
     * \param filter text filter to apply to subscription
     * \param domain domain of the network components
     * \return success flag
     */
    GRAVITY_API GravityReturnCode subscribe(std::string dataProductID, const GravitySubscriber& subscriber, 
											std::string filter = "", std::string domain = "");

    /**
     * Un-subscribe to a data product
     * \param dataProductID ID of data product for which subscription is to be removed
     * \param subscriber the subscriber that will be removed from the notification list for this subscription
     * \param filter text filter associated with the subscription to cancel
     * \paramn domain domain of the network components
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unsubscribe(std::string dataProductID, const GravitySubscriber& subscriber, 
												std::string filter="", std::string domain = "");

    /**
     * Publish a data product
     * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
     * \return success flag
     */
    GRAVITY_API GravityReturnCode publish(const GravityDataProduct& dataProduct, std::string filterText = "");

    /**
     * Make a request against a service provider through the Gravity Service Directory
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param requestor object implementing the GravityRequestor interface that will be notified of the response
     * \param requestID identifier for this request
     * \param timeout_microseconds Timeout in Microseconds (-1 for no timeout)
     * \param domain domain of the network components
     * \return success flag
     */
    GRAVITY_API GravityReturnCode request(std::string serviceID, const GravityDataProduct& request,
										const GravityRequestor& requestor, std::string requestID = "", 
										int timeout_milliseconds = -1, std::string domain = "");	

    /**
     * Make a synchronous request against a service provider
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param timeout_milliseconds Timeout in Milliseconds (-1 for no timeout)
     * \param domain domain of the network components
     * \return shared_ptr<GravityDataProduct> NULL upon failure.
     */
    GRAVITY_API shared_ptr<GravityDataProduct> request(std::string serviceID, const GravityDataProduct& request, 
										int timeout_milliseconds = -1, std::string domain = "");

    /**
     * Starts a heart beat for this gravity process.
     * \param interval_in_microseconds interval that heart beats are sent.  Typed as a signed 64 bit integer to
     * make passing to Java via Swig cleaner.
     */
    GRAVITY_API GravityReturnCode startHeartbeat(int64_t interval_in_microseconds);

    GRAVITY_API std::string getStringParam(std::string key, std::string default_value = "");
    GRAVITY_API int getIntParam(std::string key, int default_value = -1);
    GRAVITY_API double getFloatParam(std::string key, double default_value = 0.0);
    GRAVITY_API bool getBoolParam(std::string key, bool default_value = false);

    /**
     * Get the ID of this gravity node (given in the init function).
     */
    GRAVITY_API std::string getComponentID();

    /**
     * @name Registration functions
     *  These presumably must only be accessed by one thread at a time (this is true for registerHeartbeatListener,
     *   registerDataProduct doesn't synchronize access to publishMap either [besides that it's ok]).
     * @{
     */

    /**
     * Register a data product with the Gravity, and optionally, the Directory Service, making it available to the
     * rest of the Gravity-enabled system.
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param transport type (e.g. 'tcp', 'ipc')
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerDataProduct(std::string dataProductID, GravityTransportType transportType);

    /**
     * Un-register a data product, resulting in its removal from the Gravity Service Directory
     * \param dataProductID string ID used to uniquely identify this published data product
     */
    GRAVITY_API GravityReturnCode unregisterDataProduct(std::string dataProductID);
    /**
     * Register as a service provider with Gravity, and optionally, the Service Directory
     * \param serviceID Unique ID with which to register this service
     * \param transport type for requests (e.g. 'tcp', 'ipc')
     * \param server object implementing the GravityServiceProvider interface that will be notified of requests
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerService(std::string serviceID, GravityTransportType transportType,
            const GravityServiceProvider& server);
    /**
     * Unregister as a service provider with the Gravity Service Directory
     * \param serviceID Unique ID with which the service was originially registered
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unregisterService(std::string serviceID);

    /**
     * Registers a callback to be called when we don't get a heartbeat from another component.
     * \param componentID Look for heart beats from this component
     * \param interval_in_microseconds interval that heart beats are expected.  Typed as a signed 64 bit integer to
     * make passing to Java via Swig cleaner.
     * \param listener instance of a GravityHeartbeatListener that will be notified when heart beats are received or missed.
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerHeartbeatListener(std::string componentID, int64_t interval_in_microseconds, const GravityHeartbeatListener& listener);

	/**
     * Unregisters a callback for when we get a heartbeat from another component.
	 * \param componentID name of component we are currently registered to
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unregisterHeartbeatListener(std::string componentID);


    /**
     * Returns a string representation of the provided error code.
     */
    GRAVITY_API std::string getCodeString(GravityReturnCode code);

	/**
	* Utility method to get the host machine's IP address
	**/
	GRAVITY_API std::string getIP();  

    /** @} */ //Registration Functions
};

} /* namespace gravity */
#endif /* GRAVITYNODE_H_ */
