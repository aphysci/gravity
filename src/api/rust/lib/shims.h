#pragma once
#ifndef SHIMS_H_
#define SHIMS_H_

#include <string>

#include "GravityDataProduct.h"
#include "GravityNode.h"
#include "GravityRequestor.h"
// #include "RustSubscriber.h"
#include "/home/anson/gravity/src/api/rust/target/cxxbridge/rust/cxx.h"
#include "SpdLog.h"

namespace gravity
{
    class RustServiceProvider : public GravityServiceProvider {
        private:    
            rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func;
            size_t addr;
        public:
            RustServiceProvider(rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func, size_t addr);
            virtual std::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    };

    std::unique_ptr<RustServiceProvider> rustRustServiceProvider(rust::Fn<std::shared_ptr<GravityDataProduct>(const std::string&, const GravityDataProduct&, size_t)> func, size_t addr);

    GravityReturnCode rustRegisterService(const std::unique_ptr<GravityNode>& gn, const std::string& serviceID,
                            GravityTransportType transportType, const std::unique_ptr<RustServiceProvider>& server);

    GravityReturnCode rustUnregisterService(const std::unique_ptr<GravityNode>& gn, const std::string& serviceID);

    class RustRequestor : public GravityRequestor {
        private:
            rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> func;
            size_t addr;

            public:
                RustRequestor(rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> func, size_t addr);
                virtual void requestFilled(std::string serviceID, std::string requestID,
                                            const GravityDataProduct& response);
        };

    std::unique_ptr<RustRequestor> rustRustRequestor(rust::Fn<void(const std::string&, const std::string&, const GravityDataProduct&, size_t)> func, size_t addr);

    GravityReturnCode rustRequest(const std::unique_ptr<GravityNode>&gn, const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& dataProduct, 
                                const std::unique_ptr<RustRequestor>& requestor, const std::string& requestID, int timeout_milliseconds, const std::string& domain);
    
    std::shared_ptr<GravityDataProduct> rustRequestSync(const std::unique_ptr<GravityNode> &gn, const std::string& serviceID, const std::unique_ptr<GravityDataProduct>& request, int timeout_milliseconds = -1, const std::string& domain = "");

    class RustSubscriber : public GravitySubscriber {
        private:
            rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func;
            size_t addr;
        public:
            RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr);
            virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
    };

    std::unique_ptr<RustSubscriber> newRustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr);
    
    std::unique_ptr<GravityDataProduct> copyGravityDataProduct(const GravityDataProduct& gdp);

    std::shared_ptr<GravityDataProduct> copyGravityDataProductShared(const GravityDataProduct& gdp);
    

    /** Rust-able default constructor
     * 
     */
     std::unique_ptr<GravityDataProduct> newGravityDataProduct();

    /**
     * Constructor wrapper callable by rust
     * \param dataProductID string descriptor for this data product. Name by which subscribers will configure subscriptions
     * \return a pointer to a GravityDataProduct
     */
    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const std::string& dataProductId);
   
    std::unique_ptr<GravityDataProduct> newGravityDataProduct(const char *arrayPtr, int size);
   
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



    uint64_t rustGetGravityTimestamp(const std::unique_ptr<GravityDataProduct>& gdp);

    uint64_t rustGetReceivedTimestamp(const std::unique_ptr<GravityDataProduct>& gdp);

    std::unique_ptr<std::string> rustGetDataProductID(const std::unique_ptr<GravityDataProduct>& gdp);

    void rustSetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp, const std::string& softwareVersion);

    std::unique_ptr<std::string> rustGetSoftwareVersion(const std::unique_ptr<GravityDataProduct>& gdp);

    const char * rustGetProtoBytes(const std::unique_ptr<GravityDataProduct>& gdp);

    int rustGetDataSize(const std::unique_ptr<GravityDataProduct>& gdp);


    /**
     * Constructor, callable by Rust
     * \return pointer to a GravityNode
     */
    std::unique_ptr<GravityNode> newGravityNode();

    /**
	* Constructor that also initializes
	* \param componentID ID of the component to initialize
	*/
    std::unique_ptr<GravityNode> newGravityNodeId(const std::string& componentId);
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
    
    GravityReturnCode rustStartHeartbeat(const std::unique_ptr<GravityNode>& gn, int64_t interval_in_microseconds);
 
    GravityReturnCode rustStopHeartbeat(const std::unique_ptr<GravityNode>& gn);
    
    std::unique_ptr<std::string> rustGetStringParam(const std::unique_ptr<GravityNode>& gn, const std::string &key, const std::string& default_value = "");
    int rustGetIntParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, int default_value = -1);
    double rustGetFloatParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, double default_value = 0.0);
    bool rustGetBoolParam(const std::unique_ptr<GravityNode>& gn, const std::string& key, bool default_value = false);
 
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
     * Register a data product with the Gravity, and optionally, the Directory Service, making it available to the
     * rest of the Gravity-enabled system. For Rust
     * \param dataProductID string ID used to uniquely identify this published data product
     * \param transportType transport type (e.g. 'tcp', 'ipc')
	 * \param cacheLastValue flag used to signify whether or not GravityNode will cache the last sent value for a published dataproduct
     * \return success flag
     */
    GravityReturnCode rustRegisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string & dataProductID, GravityTransportType transportType, bool cacheLastValue);

    /**
     * For Rust.
     * Un-register a data product, resulting in its removal from the Gravity Service Directory
     * \param dataProductID string ID used to uniquely identify this published data product
     */
    GravityReturnCode rustUnregisterDataProduct(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID);

    /**
     * Publish a data product to the Gravity Service Directory. Callable by Rust.
     * \param dataProduct GravityDataProduct to publish, making it available to any subscribers
     * \return success flag
     */
    GravityReturnCode rustPublish(const std::unique_ptr<GravityNode> &gn, const std::unique_ptr<GravityDataProduct> &gdp);

    GravityReturnCode rustSubscribersExist(const std::unique_ptr<GravityNode> &gn, const std::string& dataProductID, bool& hasSubscribers);


    std::unique_ptr<std::string> rustGetCodeString(const std::unique_ptr<GravityNode> &gn, GravityReturnCode code);
    std::unique_ptr<std::string> rustGetIP(const std::unique_ptr<GravityNode> &gn);
    std::unique_ptr<std::string> rustGetDomain(const std::unique_ptr<GravityNode> &gn);


    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber);
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                    const std::string& filter);
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                    const std::string& filter, const std::string& domain);
    GravityReturnCode rustSubscribe(const std::unique_ptr<GravityNode>& gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                    const std::string& filter, const std::string& domain, bool recieveLastCachedValue);



    GravityReturnCode rustUnsubscribe(const std::unique_ptr<GravityNode> & gn, const std::string& dataProductId, const std::unique_ptr<RustSubscriber>& subscriber);

    GravityReturnCode rustRegisterRelay(const std::unique_ptr<GravityNode> & gn,
                                        const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                        bool localOnly, GravityTransportType transportType);

    GravityReturnCode rustRegisterRelayCache(const std::unique_ptr<GravityNode> & gn,
                                             const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber,
                                             bool localOnly, GravityTransportType transportType, bool cacheLastValue);

    GravityReturnCode rustUnregisterRelay(const std::unique_ptr<GravityNode> & gn, const std::string& dataProductID, const std::unique_ptr<RustSubscriber>& subscriber);

    /* Future response things*/
    

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