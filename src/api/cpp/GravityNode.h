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
#include "GravityServiceProvider.h"
#include "Utility.h"
#include <pthread.h>

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

using namespace std;

GRAVITY_API extern std::string emptyString; //This allows us to use default parameters inside a dll.

/**
 * Enumerated Type for Gravity Return Codes
 */
namespace GravityReturnCodes
{
    enum Codes
    {
        SUCCESS = 0,
        FAILURE = -1,
        NO_SERVICE_DIRECTORY = -2,
        REQUEST_TIMEOUT = -3,
        DUPLICATE = -4,
        REGISTRATION_CONFLICT = -5,
        NOT_REGISTERED = -6,
        NO_SUCH_SERVICE = -7,
        NO_SUCH_DATA_PRODUCT = -8,
        LINK_ERROR = -9,
        INTERRUPTED = -10,
        NO_SERVICE_PROVIDER = -11,
        NO_PORTS_AVAILABLE = -12
    };
}
typedef GravityReturnCodes::Codes GravityReturnCode;

typedef struct NetworkNode
{
    string ipAddress;
    unsigned short port;
    string transport;
    void* socket;
} NetworkNode;

GRAVITY_API uint64_t getCurrentTime(); ///< Utility method to get the current system time in epoch milliseconds

class GravityConfigParser;

/**
 * The GravityNode is a component that provides a simple interface point to a Gravity-enabled application
 */
class GravityNode
{
private:
    static const int NETWORK_TIMEOUT = 3000; // msec
    static const int NETWORK_RETRIES = 3; // attempts to connect

    pthread_t subscriptionManagerThread;
    pthread_t publishManagerThread;
    pthread_t requestManagerThread;
    pthread_t serviceManagerThread;

    void* context;
    void* subscriptionManagerSocket;
    void* requestManagerSocket;
    void* serviceManagerSocket;
    void* hbSocket; // Inproc socket for adding requests to heartbeat listener thread.
    string getIP(); ///< Utility method to get the host machine's IP address
    GravityReturnCode sendRequestToServiceDirectory(const GravityDataProduct& request, GravityDataProduct& response);
    GravityReturnCode sendRequestsToServiceProvider(string url, const GravityDataProduct& request, GravityDataProduct& response,
    		int timeout_in_milliseconds, int retries);
    GravityReturnCode sendRequestToServiceProvider(string url, const GravityDataProduct& request, GravityDataProduct& response,
    		int timeout_in_milliseconds);

    NetworkNode serviceDirectoryNode;
    map<string,string> publishMap;
    map<string,string> serviceMap; ///< Maps serviceID to url

    std::string componentID;
	GravityConfigParser* parser;

	GravityReturnCode ServiceDirectoryServiceLookup(std::string serviceOrDPID, std::string &url);
	GravityReturnCode ServiceDirectoryDataProductLookup(std::string serviceOrDPID, vector<std::string> &urls);
public:
    /**
     * Default Constructor
     */
    GRAVITY_API GravityNode();

    /**
     * Default Destructor
     */
    GRAVITY_API virtual ~GravityNode();

    /**
     * Initialize the Gravity infrastructure.
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
     * \return success flag
     */
    GRAVITY_API GravityReturnCode subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter = emptyString);

    /**
     * Setup a subscription to a data product through direct connection to known producer
     * \param connectionURL connection string of data producer
     * \param dataProductID string ID of the data product of interest
     * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
     * \param filter text filter to apply to subscription
     * \return success flag
     */
    GRAVITY_API GravityReturnCode subscribe(string connectionURL, string dataProductID,
            const GravitySubscriber& subscriber, string filter = emptyString);

    /**
     * Un-subscribe to a data product
     * \param dataProductID ID of data product for which subscription is to be removed
     * \param subscriber the subscriber that will be removed from the notification list for this subscription
     * \param filter text filter associated with the subscription to cancel
     * \return success flag
     */
    GRAVITY_API GravityReturnCode unsubscribe(string dataProductID, const GravitySubscriber& subscriber, string filter=emptyString);

    /**
     * Publish a data product
     * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
     * \return success flag
     */
    GRAVITY_API GravityReturnCode publish(const GravityDataProduct& dataProduct, std::string filterText = emptyString);

    /**
     * Make a request against a service provider through the Gravity Service Directory
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param requestor object implementing the GravityRequestor interface that will be notified of the response
     * \param requestID identifier for this request
     * \param timeout_microseconds Timeout in Microseconds (-1 for no timeout)
     * \return success flag
     */
    GRAVITY_API GravityReturnCode request(string serviceID, const GravityDataProduct& request,
            const GravityRequestor& requestor, string requestID = emptyString, int timeout_milliseconds = -1);
    /**
     * Make a request against a service provider directly
     * \param connectionURL connection string on which service provider is listening for requests
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param requestor object implementing the GravityRequestor interface that will be notified of the response
     * \param requestID identifier for this request
     * \param timeout_microseconds Timeout in Microseconds (-1 for no timeout)
     * \return success flag
     */
    GRAVITY_API GravityReturnCode request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
            const GravityRequestor& requestor, string requestID = emptyString, int timeout_milliseconds = -1);

    /**
     * Make a synchronous request against a service provider
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param timeout_microseconds Timeout in Microseconds (-1 for no timeout)
     * \return shared_ptr<GravityDataProduct> NULL upon failure.
     */
    GRAVITY_API shared_ptr<GravityDataProduct> request(string serviceID, const GravityDataProduct& request, int timeout_milliseconds = -1);

    /**
     * Starts a heart beat for this gravity process.
     */
    GRAVITY_API GravityReturnCode startHeartbeat(int interval_in_microseconds);

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
     * \param addToDirectory true if this data product should be registered in the ServiceDirectory
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerDataProduct(string dataProductID, string transportType, bool addToDirectory = true);

    /**
     * Un-register a data product, resulting in its removal from the Gravity Service Directory
     */
    GRAVITY_API GravityReturnCode unregisterDataProduct(string dataProductID);
    /**
     * Register as a service provider with Gravity, and optionally, the Service Directory
     * \param serviceID Unique ID with which to register this service
     * \param networkPort network port on which this provider will listen for requests
     * \param transport type for requests (e.g. 'tcp', 'ipc')
     * \param server object implementing the GravityServiceProvider interface that will be notified of requests
     * \param addToDirectory true if this data product should be registered in the ServiceDirectory
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerService(string serviceID, unsigned short networkPort,
            string transportType, const GravityServiceProvider& server, bool addToDirectory = true);
    /**
     * Register as a service provider with Gravity, and optionally, the Service Directory
     * \param serviceID Unique ID with which to register this service
     * \param connectionURL connection string on which service provider will listen for requests
     * \param server object implementing the GravityServiceProvider interface that will be notified of requests
     * \param addToDirectory true if this data product should be registered in the ServiceDirectory
     * \return success flag
     */
    GRAVITY_API GravityReturnCode registerService(string serviceID, string connectionURL,
    		const GravityServiceProvider& server, bool addToDirectory = true);

    /**
     * Unregister as a service provider with the Gravity Service Directory
     * \param serviceID Unique ID with which the service was originially registered
     */
    GRAVITY_API GravityReturnCode unregisterService(string serviceID);

    /**
     * Registers a callback to be called when we don't get a heartbeat from another component.
     */
    GRAVITY_API GravityReturnCode registerHeartbeatListener(string componentID, uint64_t timebetweenMessages, const GravityHeartbeatListener& listener);

    /** @} */ //Registration Functions
};

} /* namespace gravity */
#endif /* GRAVITYNODE_H_ */
