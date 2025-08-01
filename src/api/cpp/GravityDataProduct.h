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
 * GravityDataProduct.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYDATAPRODUCT_H_
#define GRAVITYDATAPRODUCT_H_

// #ifdef __GNUC__
// #include <memory>
// #else
#include <memory>
// #endif
#include "protobuf/GravityDataProductPB.pb.h"
#include "Utility.h"

namespace gravity
{

class GravityNode;

/**
 * Generic Data Product for the Gravity Infrastructure
 */
class GRAVITY_API GravityDataProduct
{
protected:
    std::shared_ptr<GravityDataProductPB> gravityDataProductPB;  ///< internal protobuf representation of data product
    friend class GravityNode;
    friend class GravityMetricsManager;
    friend class GravityServiceManager;
    friend void* Heartbeat(void*);

public:
    /**
     * Default Constructor
     */
    GravityDataProduct() {}

    /**
     * Constructor
     * \param dataProductID string descriptor for this data product. Name by which subscribers will configure subscriptions
     * \return a GravityDataProduct
     */
    GravityDataProduct(std::string dataProductID);

    /**
     * Constructor that deserializes this GravityDataProduct from array of bytes
     * \param arrayPtr pointer to array of bytes containing serialized GravityDataProduct
     * \param size size of serialized data
     * \return a GravityDataProduct
     */
    GravityDataProduct(const void* arrayPtr, int size);

    /**
     * Default Destructor
     */
    virtual ~GravityDataProduct();

    /**
     * Method to return the timestamp associated with this data product.
     *  Represents Microseconds since the Unix Epoch
     * \return timestamp for data
     */
    uint64_t getGravityTimestamp() const;

    /**
     * Method to return the timestamp associated with the receipt of this data product
     *  Represents Microseconds since the Unix Epoch
     * \return received timestamp for data (0 if not received via subscription)
     */
    uint64_t getReceivedTimestamp() const;

    /**
     * Method to return the data product ID for this data
     * \return data product ID
     */
    std::string getDataProductID() const;

    /**
     * Setter for the specification of software version information
     * \param softwareVersion string specifying the version number
     */
    void setSoftwareVersion(std::string softwareVersion);

    /**
     * Getter for the software version specified on this data product
     * \return the software version number associated with this data product
     */
    std::string getSoftwareVersion() const;

    /**
     * Set the application-specific data for this data product
     * \param data pointer to arbitrary data
     * \param size length of data
     */
    void setData(const void* data, int size);

    /**
     * Set the application-specific data for this data product
     * \param data A Google Protocol Buffer Message object containing the data
     */
    void setData(const google::protobuf::Message& data);

    /**
     * Getter for the application-specific data contained within this data product
     * \param data pointer to array to populate with message data content
     * \param size size of data array to populate
     * \return success flag
     */
    bool getData(void* data, int size) const;

    /**
     * Get the size of the data contained within this data product
     * \return size in bytes of contained data
     */
    int getDataSize() const;

    /**
     * Populate a protobuf object with the data contained in this data product
     * \param data Google Protocol Buffer Message object to populate
     * \return success flag
     */
    bool populateMessage(google::protobuf::Message& data) const;

    /**
     * Get the size for this message
     * \return size in bytes for this data product
     */
    int getSize() const;

    /**
     * Deserialize this GravityDataProduct from array of bytes
     * \param arrayPtr pointer to array of bytes containing serialized GravityDataProduct
     * \param size size of serialized data
     */
    void parseFromArray(const void* arrayPtr, int size);

    /**
     * Serialize this GravityDataProduct
     * \param arrayPtr pointer to array into which to serialize this GravityDataProduct
     * \return success flag
     */
    bool serializeToArray(void* arrayPtr) const;

    /**
     * Check equivalence between two GravityDataProducts.  GravityDataProducts are considered equivalent
     * if their product IDs and data are identical.
     * \param gdp GravityDataProduct to compare with this one.
     * \return true if the two GravityDataProducts are equivalent, false otherwise.
     */
    bool operator==(const GravityDataProduct& gdp) const;

    /**
     * Check non-equivalence between two GravityDataProducts. 
     * \param gdp GravityDataProduct to compare with this one.
     * \return true if the two GravityDataProducts are not equivalent, false otherwise.
     */
    bool operator!=(const GravityDataProduct& gdp) const;

    /**
	 * Get the component ID of the GravityNode that produced this data product
	 * \return component ID of the source GravityNode
	 */
    std::string getComponentId() const;

    /**
	 * Get the domain of the GravityNode that produced this data product
	 * \return domain of the source GravityNode
	 */
    std::string getDomain() const;

    /**
	 * Get the flag indicating if this is a "future" response
	 * \return future response flag
	 */
    bool isFutureResponse() const;

    /**
	* Get the flag indicated if this was a cached data product.	
	*/
    bool isCachedDataproduct() const;

    /**
	* Sets the flag indicating this data product is cached	
	*/
    void setIsCachedDataproduct(bool cached);

    /**
	 * Get the url for the REP socket of a future response
	 * \return url for REP socket of future response
	 */
    std::string getFutureSocketUrl() const;

    /**
     * Set the timestamp on this GravityDataProduct (typically set by infrastructure at publish)
     * \param ts Timestamp (epoch microseconds) for this GravityDataProduct
     */
    void setTimestamp(uint64_t ts) const { gravityDataProductPB->set_timestamp(ts); }

    /**
     * Set the received timestamp on this GravityDataProduct (typically set by infrastructure on receipt)
     * \param ts Received timestamp (epoch microseconds) for this GravityDataProduct
     */
    void setReceivedTimestamp(uint64_t ts) const { gravityDataProductPB->set_received_timestamp(ts); }

    /**
     * Set the component id on this GravityDataProduct (typically set by infrastructure at publish)
     * \param componentId ID of the component that produces this GravityDataProduct
     */
    void setComponentId(std::string componentId) const
    {
        gravityDataProductPB->set_componentid(componentId);
    }

    /**
     * Set the domain on this GravityDataProduct (typically set by infrastructure at publish)
     * \param domain name of the domain on which this GravityDataProduct is produced
     */
    void setDomain(std::string domain) const { gravityDataProductPB->set_domain(domain); }

    /**
	 * Get the flag indicating if this message has been relayed by a Relay component
	 */
    bool isRelayedDataproduct() const;

    /**
	 * Set the flag indicating whether this message has been relayed or has come from the original source
	 */
    void setIsRelayedDataproduct(bool relayed);

    /**
	 * Set the data protocol in this data product (for instance, protobuf2)
	 */
    void setProtocol(const std::string& protocol);

    /**
	 * Get the data protocol in this data product (for instance, protobuf2)
	 */
    const std::string& getProtocol() const;

    /**
	 * Set the type of data in this data product (for instance, the full protobuf type name)
	 */
    void setTypeName(const std::string& dataType);

    /**
	 * Get the type of data in this data product (for instance, the full protobuf type name)
	 */
    const std::string& getTypeName() const;

    /**
	* Method to return the time associated with the registration of this data product
	*  Represents Seconds since the Unix Epoch
	* \return registration time for data (0 if not set)
	*/
    uint32_t getRegistrationTime() const;

    /**
	* Set the registration time on this GravityDataProduct (typically set by infrastructure when created)
	* \param ts Registration time (epoch seconds) for this GravityDataProduct
	*/
    void setRegistrationTime(uint32_t ts) const { gravityDataProductPB->set_registration_time(ts); }
};

} /* namespace gravity */
#endif /* GRAVITYDATAPRODUCT_H_ */
