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
#include "GravityServiceProvider.h"

namespace gravity
{

using namespace std;

/**
 * The GravityNode is a component that provides a simple interface point to a Gravity-enabled application
 */
class GravityNode
{
private:
	uint64_t getCurrentTime(); ///< Utility method to get the current system time in epoch milliseconds
	string getIP(); ///< Utility method to get the host machine's IP address
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
	 * Register a data product with the Gravity Directory Service, making it available to the
	 * rest of the Gravity-enabled system.
	 * \param dataProductID string ID used to uniquely identify this published data product
	 * \param networkPort network port on which this data product is made available
	 * \param transport type (e.g. 'tcp', 'ipc')
	 * \return success flag
	 */
	bool registerDataProduct(string dataProductID, unsigned short networkPort, string transportType);

	/**
	 * Un-register a data product, resulting in its removal from the Gravity Service Directory
	 */
	bool unregisterDataProduct(string dataProductID);

	/**
	 * Setup a subscription to a data product through the Gravity Service Directory.
	 * \param dataProductID string ID of the data product of interest
	 * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
	 * \param filter text filter to apply to subscription
	 * \return success flag
	 */
	bool subscribe(string dataProductID, const GravitySubscriber& subscriber, string filter = "");

	/**
	 * Setup a subscription to a data product through direct connection to known producer
	 * \param connectionURL connection string of data producer
	 * \param dataProductID string ID of the data product of interest
	 * \param subscriber object that implements the GravitySubscriber interface and will be notified of data availability
	 * \param filter text filter to apply to subscription
	 * \return success flag
	 */
	bool subscribe(string connectionURL, string dataProductID,
				   const GravitySubscriber& subscriber, string filter = "");

	/**
	 * Un-subscribe to a data product
	 * \param dataProductID ID of data product for which subscription is to be removed
	 * \param subscriber the subscriber that will be removed from the notification list for this subscription
	 * \return success flag
	 */
	bool unsubscribe(string dataProductID, const GravitySubscriber& subscriber);

	/**
	 * Publish a data product
	 * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
	 * \return success flag
	 */
	bool publish(const GravityDataProduct& dataProduct);

	/**
	 * Make a request against a service provider through the Gravity Service Directory
	 * \param serviceID The registered service ID of a service provider
	 * \param dataProduct data product representation of the request
	 * \param requestor object implementing the GravityRequestor interface that will be notified of the response
	 * \param requestID identifier for this request
	 * \return success flag
	 */
	bool request(string serviceID, const GravityDataProduct& dataProduct,
				 const GravityRequestor& requestor, string requestID = "");

	/**
	 * Make a request against a service provider directly
	 * \param connectionURL connection string on which service provider is listening for requests
	 * \param serviceID The registered service ID of a service provider
	 * \param dataProduct data product representation of the request
	 * \param requestor object implementing the GravityRequestor interface that will be notified of the response
	 * \param requestID identifier for this request
	 * \return success flag
	 */
	bool request(string connectionURL, string serviceID, const GravityDataProduct& dataProduct,
				 const GravityRequestor& requestor, string requestID = "");

	/**
	 * Register as a service provider with the Gravity Service Directory
	 * \param serviceID Unique ID with which to register this service
	 * \param networkPort network port on which this provider will listen for requests
	 * \param transport type for requests (e.g. 'tcp', 'ipc')
	 * \param server object implementing the GravityServiceProvider interface that will be notified of requests
	 * \return success flag
	 */
	bool registerService(string serviceID, unsigned short networkPort,
						 string transportType, const GravityServiceProvider& server);

	/**
	 * Unregister as a service provider with the Gravity Service Directory
	 * \param serviceID Unique ID with which the service was originially registered
	 * \param server the GravityServiceProvider object that will no longer receive requests
	 */
	bool unregisterService(string serviceID, const GravityServiceProvider& server);
};

} /* namespace gravity */
#endif /* GRAVITYNODE_H_ */
