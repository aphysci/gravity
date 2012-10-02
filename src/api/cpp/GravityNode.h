/*
 * GravityNode.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYNODE_H_
#define GRAVITYNODE_H_

#include "protobuf/ServiceDirectoryRegistrationPB.pb.h"
#include "GravitySubscriber.h"
#include "GravityRequestor.h"
#include "GravityHeartbeatListener.h"
#include "GravityServiceProvider.h"
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

extern std::string emptyString; //This allows us to use default parameters inside a dll.

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
        NO_SERVICE_PROVIDER = -11
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

uint64_t getCurrentTime(); ///< Utility method to get the current system time in epoch milliseconds

/**
 * The GravityNode is a component that provides a simple interface point to a Gravity-enabled application
 */
class GravityNode
{
private:
    static const int NETWORK_TIMEOUT = 3000; // msec
    static const int NETWORK_RETRIES = 3; // attempts to connect

    pthread_t subscriptionManagerThread;
    pthread_t requestManagerThread;
    pthread_t serviceManagerThread;

    void* context;
    void* subscriptionManagerSocket;
    void* requestManagerSocket;
    void* serviceManagerSocket;
    void* hbSocket; // Inproc socket for adding requests to heartbeat listener thread.
    void sendStringMessage(void* socket, string str, int flags);
    string readStringMessage(void* socket);
    string getIP(); ///< Utility method to get the host machine's IP address
    void sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct);
    GravityReturnCode sendRequestToServiceDirectory(const GravityDataProduct& request, GravityDataProduct& response);
    GravityReturnCode sendRequestToServiceProvider(string url, const GravityDataProduct& request, GravityDataProduct& response);
    NetworkNode serviceDirectoryNode;
    map<string,NetworkNode*> publishMap;
    map<string,string> serviceMap;

public:
    /**
     * Default Constructor
     */
    GravityNode();

    /**
     * Default Destructor
     */
    virtual ~GravityNode();

    /**
     * Initialize the Gravity infrastructure.
     * \return GravityReturnCode code to identify any errors that occur during initialization
     */
    GravityReturnCode init(); //TODO: add service directory as a parameter to this guy!

    /**
     * Wait for the GravityNode to exit.
     */
    void waitForExit();

    /**
     * Setup a subscription to a data product through the Gravity Service Directory.
     * \param dataProductID string ID of the data product of interest
     * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
     * \param filter text filter to apply to subscription
     * \return success flag
     */
    GravityReturnCode subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter = emptyString);

    /**
     * Setup a subscription to a data product through direct connection to known producer
     * \param connectionURL connection string of data producer
     * \param dataProductID string ID of the data product of interest
     * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
     * \param filter text filter to apply to subscription
     * \return success flag
     */
    GravityReturnCode subscribe(string connectionURL, string dataProductID,
            const GravitySubscriber& subscriber, string filter = emptyString);

    /**
     * Un-subscribe to a data product
     * \param dataProductID ID of data product for which subscription is to be removed
     * \param subscriber the subscriber that will be removed from the notification list for this subscription
     * \param filter text filter associated with the subscription to cancel
     * \return success flag
     */
    GravityReturnCode unsubscribe(string dataProductID, const GravitySubscriber& subscriber, string filter=emptyString);

    /**
     * Publish a data product
     * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
     * \return success flag
     */
    GravityReturnCode publish(const GravityDataProduct& dataProduct, std::string filterText = emptyString);

    /**
     * Make a request against a service provider through the Gravity Service Directory
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param requestor object implementing the GravityRequestor interface that will be notified of the response
     * \param requestID identifier for this request
     * \return success flag
     */
    GravityReturnCode request(string serviceID, const GravityDataProduct& dataProduct,
            const GravityRequestor& requestor, string requestID = emptyString);

    /**
     * Make a request against a service provider directly
     * \param connectionURL connection string on which service provider is listening for requests
     * \param serviceID The registered service ID of a service provider
     * \param dataProduct data product representation of the request
     * \param requestor object implementing the GravityRequestor interface that will be notified of the response
     * \param requestID identifier for this request
     * \return success flag
     */
    GravityReturnCode request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
            const GravityRequestor& requestor, string requestID = emptyString);

    /**
     * Starts a heartbeat for this gravity process.
     */
    GravityReturnCode startHeartbeat(std::string componentID, int interval_in_microseconds, unsigned short port = 54541);

    /**
     * @name Registration functions
     *  These presumably must only be accessed by one thread at a time (this is true for registerHeartbeatListener,
     *   registerDataProduct doesn't synchronize access to publishMap either [besides that it's ok]).
     * @{
     */

    /**
     * Register a data product with the Gravity Directory Service, making it available to the
     * rest of the Gravity-enabled system.
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param networkPort network port on which this data product is made available
     * \param transport type (e.g. 'tcp', 'ipc')
     * \return success flag
     */
    GravityReturnCode registerDataProduct(string dataProductID, unsigned short networkPort, string transportType);

    /**
     * Un-register a data product, resulting in its removal from the Gravity Service Directory
     */
    GravityReturnCode unregisterDataProduct(string dataProductID);
    /**
     * Register as a service provider with the Gravity Service Directory
     * \param serviceID Unique ID with which to register this service
     * \param networkPort network port on which this provider will listen for requests
     * \param transport type for requests (e.g. 'tcp', 'ipc')
     * \param server object implementing the GravityServiceProvider interface that will be notified of requests
     * \return success flag
     */
    GravityReturnCode registerService(string serviceID, unsigned short networkPort,
            string transportType, const GravityServiceProvider& server);

    /**
     * Unregister as a service provider with the Gravity Service Directory
     * \param serviceID Unique ID with which the service was originially registered
     */
    GravityReturnCode unregisterService(string serviceID);

    /**
     * Registers a callback to be called when we don't get a heartbeat from another component.
     */
    GravityReturnCode registerHeartbeatListener(string dataProductID, uint64_t timebetweenMessages, const GravityHeartbeatListener& listener);

    /** @} */ //Registration Functions
};

} /* namespace gravity */
#endif /* GRAVITYNODE_H_ */
