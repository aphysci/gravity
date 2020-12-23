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

/**
 *
 */
package com.aphysci.gravity;

import com.aphysci.gravity.protobuf.GravityDataProductContainer.GravityDataProductPB;
import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.Message;


/**
 * @author mbarger
 *
 */
public class GravityDataProduct {

	protected GravityDataProductPB.Builder gdp = GravityDataProductPB.newBuilder();

    /**
     * Constructor
     * @param dataProductID string descriptor for this data product. Name by which subscribers will configure subscriptions
     */
    public GravityDataProduct(String dataProductID) {
    	gdp.setDataProductID(dataProductID);
    }

    /**
     * Constructor that deserializes this GravityDataProduct from array of bytes
     * @param arrayPtr pointer to array of bytes containing serialized GravityDataProduct
     * @param size size of serialized data
     */
    public GravityDataProduct(byte[] data) {
    	try {
			gdp.mergeFrom(data);
		} catch (InvalidProtocolBufferException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }

    /**
     * Method to return the timestamp associated with this data product
     * @return timestamp for data
     */
    public long getGravityTimestamp() {
    	return gdp.getTimestamp();
    }
    
	/**
     * Method to return the timestamp associated with receipt of data product
     * @return received timestamp for data
     */
    public long getReceivedTimestamp() {
		long receivedTimestamp = 0;
		if (gdp.hasReceivedTimestamp())
		{
			receivedTimestamp = gdp.getReceivedTimestamp();
		}
    	return receivedTimestamp;
    }
    
    /**
     * Method to set the timestamp associated with receipt of data product
     * @param ts timestamp (epoch microseconds) for this GravityDataProduct's receipt timestamp
     */
    public void setReceivedTimestamp(long ts) {
		gdp.setReceivedTimestamp(ts);		
    }

    /**
     * Method to set the timestamp for this GravityDataProduct
     * @param ts timestamp (epoch microseconds) for this GravityDataProduct
     */
    public void setTimestamp(long ts)
    {
        gdp.setTimestamp(ts);
    }

    /**
     * Method to return the data product ID for this data
     * @return data product ID
     */
    public String getDataProductID() {
    	return gdp.getDataProductID();
    }

    /**
     * Setter for the specification of software version information
     * @param softwareVersion string specifying the version number
     */
    public void setSoftwareVersion(String softwareVersion) {
    	gdp.setSoftwareVersion(softwareVersion);
    }

    /**
     * Getter for the software version specified on this data product
     * @return the software version number associated with this data product
     */
    public String getSoftwareVersion() {
    	return gdp.getSoftwareVersion();
    }

    /**
     * Set the application-specific data for this data product
     * @param data pointer to arbitrary data
     * @param size length of data
     */
    public void setData(byte[] data) {
    	gdp.setData(ByteString.copyFrom(data));
    }

    /**
     * Set the application-specific data for this data product
     * @param data A Google Protocol Buffer Message object containing the data
     */
    public void setData(Message.Builder data) {
    	gdp.setData(data.build().toByteString());
    }

    /**
     * Getter for the application-specific data contained within this data product
     * @return byte array containing data, or null if no data is available.
     */
    public byte[] getData() {
    	if (gdp != null) {
    		return gdp.getData().toByteArray();
    	}
    	return null;
    }

    /**
     * Get the size of the data contained within this data product
     * @return size in bytes of contained data
     */
    public int getDataSize() {
    	if (gdp.getData() != null) {
    		return gdp.getData().size();
    	}
    	return 0;
    }

    /**
     * Populate a protobuf object with the data contained in this data product
     * @param data Google Protocol Buffer Message object to populate
     * @return success flag
     */
    public boolean populateMessage(Message.Builder data) {
    	try
    	{
    		data.mergeFrom(getData());
    		return true;
    	}
    	catch(InvalidProtocolBufferException e)
    	{
    		return false;
    	}
    }

    /**
     * Get the size for this message
     * @return size in bytes for this data product
     */
    public int getSize() {
    	return gdp.build().toByteArray().length;
    }

    /**
     * Deserialize this GravityDataProduct from array of bytes
     * @param arrayPtr pointer to array of bytes containing serialized GravityDataProduct
     * @param size size of serialized data
     */
    public void parseFromArray(byte[] data) {
    	try {
			gdp.mergeFrom(data);
		} catch (InvalidProtocolBufferException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }

    /**
     * Serialize this GravityDataProduct
     * \arrayPtr pointer to array into which to serialize this GravityDataProduct
     * @return success flag
     */
    public byte[] serializeToArray() {
    	return gdp.build().toByteArray();
    }
	
	/**
	 * Gets the componentID of the Gravity Node that produced this data product.
	 @return ComponentID of the source Gravity Node.
	 */
	public String getComponentID() {
		return gdp.getComponentID();
	}
	
	/**
	 * Gets the Domain of the Gravity Node that produced this data product.
	 @return Domain of the source Gravity Node.
	 */
	public String getDomain() {
		return gdp.getDomain();
	}

    /**
     * Method to set the component id for this GravityDataProduct (typically set by infrastructure at publish)
     * @param componentID id of component associated with this GravityDataProduct (typically producer)
     */
    public void setComponentID(String componentID)
    {
		gdp.setComponentID(componentID);
    }
    
	/**
     * Method to set the domain for this GravityDataProduct (typically set by infrastructure at publish)
     * @param domain associated with this GravityDataProduct (typically producer's domain)
     */
    public void setDomain(String domain)
    {
		gdp.setDomain(domain);
    }

	/**
	 * Method to get the future socket url 
	 * @return url of the future socket url
	 */
	public String getFutureSocketUrl()
	{
        return gdp.getFutureSocketUrl();
	}

	/**
	 * Method to get the boolean indication of whether this is a FutureResponse
	 * @return boolean flag indicating that this is a FutureResponse
	 */
	public boolean isFutureResponse()
	{
		return gdp.getFutureResponse();
	}

	/**
	 * Method to get the boolean indication of whether this is a cached data product.
	 * Unless specified otherwise, publishers cache the last data product sent to 
	 * send to new subscribers. This flag indicates whether the DP is new or if it was 
	 * sent from that cache.
	 * @return boolean flag indicating that this is a cached data product
	 */
	public boolean isCachedDataproduct()
	{
		return gdp.hasIsCachedDataproduct() && gdp.getIsCachedDataproduct();
	}

	public void setIsCachedDataproduct(boolean isCached){
		gdp.setIsCachedDataproduct(isCached);
	}
}
