#pragma once
#ifndef SHIMS_H_
#define SHIMS_H_

#include <string>

#include "GravityDataProduct.h"
#include "GravityNode.h"
#include "RustSubscriber.h"
#include "SpdLog.h"

namespace gravity
{
    
    /**
     * Constructor wrapper callable by rust
     * \param dataProductID string descriptor for this data product. Name by which subscribers will configure subscriptions
     * \return a pointer to a GravityDataProduct
     */
    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const std::string& dataProductId);
    /**
     * setData callable by rust
     * Set the application-specific data for this data product
     * \param data pointer to arbitrary data
     * \param size length of data
     */
    void rustSetData(const std::unique_ptr<GravityDataProduct> &gdp, const char *data, int size);
    /**
     * Set the application-specific data for this data product, callable by rust
     * \param data A Google Protocol Buffer Message object containing the data
     */
    void rustSetDataProto(const std::unique_ptr<GravityDataProduct> &gdp, const char* data, int size);

    /**
     * Constructor, callable by Rust
     * \return pointer to a GravityNode
     */
    std::unique_ptr<GravityNode> newGravityNode();

    /**
	* Constructor that also initializes
	* \param componentID ID of the component to initialize
	*/
    std::unique_ptr<GravityNode> newGravityNode(const std::string& componentId);
     /**
     * Initialize the Gravity infrastructure.
	   * Reads the ComponentID from the Gravity.ini file.
     * \return GravityReturnCode code to identify any errors that occur during initialization
     */
    GravityReturnCode rustInit(const std::unique_ptr<GravityNode>& gn);

    /**
     * Initialize the Gravity infrastructure. Callable by Rust.
     * \copydetails GravityNode(std::string)
     * \return GravityReturnCode code to identify any errors that occur during initialization
     */
    GravityReturnCode rustInit(const std::unique_ptr<GravityNode> &gn, const std::string &componentID);
    
    /**
     * Wait for the GravityNode to exit.
     */
    void rustWaitForExit(const std::unique_ptr<GravityNode>& gn);
    /**
     * Get the ID of this gravity node (given in the init function). Callable by Rust.
     */
    std::unique_ptr<std::string> rustGetComponentID(const std::unique_ptr<GravityNode>& gn);
    
    /**
     * Register a data product with the Gravity, and optionally, the Directory Service, making it available to the
     * rest of the Gravity-enabled system.
     * Callable by Rust.
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param transportType transport type (e.g. 'tcp', 'ipc')
     * \return success flag
     */
    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string & dataProductID, GravityTransportType transportType);
    
    /**
     * Publish a data product to the Gravity Service Directory. Callable by Rust.
     * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
     * \return success flag
     */
    GravityReturnCode rustPublish(const std::unique_ptr<GravityNode> &gn, const std::unique_ptr<GravityDataProduct> &gdp);

    GravityReturnCode rustSubscribersExist(const std::unique_ptr<GravityNode> &gn, const std::string& dataProductID, bool& hasSubscribers);
} // namespace gravity

void spdlog_critical(const std::string& message) {
    gravity::SpdLog::critical(message.c_str());
}
void spdlog_error(const std::string& message) {
    gravity::SpdLog::error(message.c_str());
}
void spdlog_warn(const std::string& message) {
    gravity::SpdLog::warn(message.c_str());
}
void spdlog_info(const std::string& message) {
    gravity::SpdLog::info(message.c_str());
}
void spdlog_debug(const std::string& message) {
    gravity::SpdLog::debug(message.c_str());
}
void spdlog_trace(const std::string& message) {
    gravity::SpdLog::trace(message.c_str());
}

#endif