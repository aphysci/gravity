/*
 * GravityDataProduct.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYDATAPRODUCT_H_
#define GRAVITYDATAPRODUCT_H_

#include <tr1/memory>
#include "GravityDataProductPB.pb.h"

namespace gravity
{

using namespace std;
using namespace std::tr1;

/**
 * Generic Data Product for the Gravity Infrastructure
 */
class GravityDataProduct
{
private:
	shared_ptr<GravityDataProductPB> gravityDataProductPB; ///< internal protobuf representation of data product
	string filterText; ///< text string on which subscribers of this data product can apply a filter
public:
	/**
	 * Constructor
	 * \param dataProductID string descriptor for this data product. Name by which subscribers will configure subscriptions
	 */
	GravityDataProduct(string dataProductID);

	/**
	 * Default Destructor
	 */
	virtual ~GravityDataProduct();

	/**
	 * Method to return the timestamp associated with this data product
	 * \return timestamp for data
	 */
	uint64_t getGravityTimestamp() const;

	/**
	 * Method to return the data product ID for this data
	 * \return data product ID
	 */
	string getDataProductID() const;

	/**
	 * Setter for the specification of software version information
	 * \param softwareVersion string specifying the version number
	 */
	void setSoftwareVersion(string softwareVersion);

	/**
	 * Getter for the software version specified on this data product
	 * \return the software version number associated with this data product
	 */
	string getSoftwareVersion() const;

	/**
	 * Setter for the filter-able text associated with this data product
	 * \param filterText text string that can be filtered against by subscribers
	 */
	void setFilterText(string filterText);

	/**
	 * Getter for the filter-able text associated with this data product
	 * \return filterText text string that can be filtered against by subscribers
	 */
	string getFilterText() const;

	/**
	 * Set the application-specific data for this data product
	 * \param data pointer to arbitrary data
	 * \param size length of data
	 */
	void setData(void* data, int size);

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
	bool populateMessage(google::protobuf::Message& data);

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
	void parseFromArray(void* arrayPtr, int size);

	/**
	 * Serialize this GravityDataProduct
	 * \arrayPtr pointer to array into which to serialize this GravityDataProduct
	 * \return success flag
	 */
	bool serializeToArray(void* arrayPtr) const;
};

} /* namespace gravity */
#endif /* GRAVITYDATAPRODUCT_H_ */
