/*
 * GravityDataProduct.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYDATAPRODUCT_H_
#define GRAVITYDATAPRODUCT_H_

#ifdef __GNUC__
#include <tr1/memory>
#else
#include <memory>
#endif
#include "protobuf/GravityDataProductPB.pb.h"
#include "Utility.h"

namespace gravity
{

using namespace std;
using namespace std::tr1;

class GravityNode;

/**
 * Generic Data Product for the Gravity Infrastructure
 */
class GravityDataProduct
{
private:
    shared_ptr<GravityDataProductPB> gravityDataProductPB; ///< internal protobuf representation of data product
    friend class GravityNode;
    friend void* Heartbeat(void*);
    //In Microseconds
    void setTimestamp(uint64_t ts) const { gravityDataProductPB->set_timestamp(ts); } //Yeah, I'm telling the compiler this is const.
public:
    GRAVITY_API GravityDataProduct() {}
    /**
     * Constructor
     * \param dataProductID string descriptor for this data product. Name by which subscribers will configure subscriptions
     */
    GRAVITY_API GravityDataProduct(string dataProductID);

    /**
     * Constructor that deserializes this GravityDataProduct from array of bytes
     * \param arrayPtr pointer to array of bytes containing serialized GravityDataProduct
     * \param size size of serialized data
     */
    GRAVITY_API GravityDataProduct(void* arrayPtr, int size);

    /**
     * Default Destructor
     */
    GRAVITY_API virtual ~GravityDataProduct();

    /**
     * Method to return the timestamp associated with this data product
     *  Represents Microseconds since the Unix Epoch
     * \return timestamp for data
     */
    GRAVITY_API uint64_t getGravityTimestamp() const;

    /**
     * Method to return the data product ID for this data
     * \return data product ID
     */
    GRAVITY_API string getDataProductID() const;

    /**
     * Setter for the specification of software version information
     * \param softwareVersion string specifying the version number
     */
    GRAVITY_API void setSoftwareVersion(string softwareVersion);

    /**
     * Getter for the software version specified on this data product
     * \return the software version number associated with this data product
     */
    GRAVITY_API string getSoftwareVersion() const;

    /**
     * Set the application-specific data for this data product
     * \param data pointer to arbitrary data
     * \param size length of data
     */
    GRAVITY_API void setData(const void* data, int size);

    /**
     * Set the application-specific data for this data product
     * \param data A Google Protocol Buffer Message object containing the data
     */
    GRAVITY_API void setData(const google::protobuf::Message& data);

    /**
     * Getter for the application-specific data contained within this data product
     * \param data pointer to array to populate with message data content
     * \param size size of data array to populate
     * \return success flag
     */
    GRAVITY_API bool getData(void* data, int size) const;

    /**
     * Get the size of the data contained within this data product
     * \return size in bytes of contained data
     */
    GRAVITY_API int getDataSize() const;

    /**
     * Populate a protobuf object with the data contained in this data product
     * \param data Google Protocol Buffer Message object to populate
     * \return success flag
     */
    GRAVITY_API bool populateMessage(google::protobuf::Message& data) const;

    /**
     * Get the size for this message
     * \return size in bytes for this data product
     */
    GRAVITY_API int getSize() const;

    /**
     * Deserialize this GravityDataProduct from array of bytes
     * \param arrayPtr pointer to array of bytes containing serialized GravityDataProduct
     * \param size size of serialized data
     */
    GRAVITY_API void parseFromArray(void* arrayPtr, int size);

    /**
     * Serialize this GravityDataProduct
     * \arrayPtr pointer to array into which to serialize this GravityDataProduct
     * \return success flag
     */
    GRAVITY_API bool serializeToArray(void* arrayPtr) const;
};

} /* namespace gravity */
#endif /* GRAVITYDATAPRODUCT_H_ */
