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
 * GravityMetrics.h
 *
 *  Created on: Feb 7, 2013
 *      Author: Chris Brundick
 */

#ifndef GRAVITYMETRICS_H_
#define GRAVITYMETRICS_H_

#include "Utility.h"
#include <map>
#include <string>
#include <vector>

namespace gravity
{

/**
 * Generic metrics data for the Gravity Infrastructure.
 * Contains general metrics data and metrics data specific to each data product.
 */
class GravityMetrics
{
private:
    typedef struct MetricsSample
    {
        int messageCount;
        int byteCount;
    } MetricsSample;

    std::map<std::string, MetricsSample> metrics;
    uint64_t startTime;
    uint64_t endTime;
public:
    /**
     * Creates an empty GravityMetrics object
     */
    GRAVITY_API GravityMetrics();

    /**
     * Creates a GravityMetrics object and populates it from
     * incoming messages on the provided socket
     * \param socket zmq_socket on which there is an incoming metrics information 
     */
    GRAVITY_API GravityMetrics(void* socket);

    /**
     * Default Destructor
     */
    GRAVITY_API virtual ~GravityMetrics();

    /**
     * Increment the message count metric for the given data product ID.
     * \param dataProductID data product ID for which the message count is to be incremented
     * \param count amount by which to increment the message count
     */
    GRAVITY_API void incrementMessageCount(std::string dataProductID, int count);

    /**
     * Increment the byte count metric for the given data product ID.
     * \param dataProductID data product ID for which the byte count is to be incremented
     * \param count amount by which to increment the byte count
     */
    GRAVITY_API void incrementByteCount(std::string dataProductID, int count);

    /**
     * Reset the metrics. This will reset all counts to zero and set the startTime for each
     * sample to the current time but maintain list of data product IDs
     */
    GRAVITY_API void reset();

    /**
     * Clear the metrics. This will clear all metrics data and data product IDs.
     */
    GRAVITY_API void clear();

    /**
     * Remove the tracking of a specific data product id
     * \param dataProductID id of data product to remove from metrics
     */
    GRAVITY_API void remove(std::string dataProductID);

    /**
     * Method to return the message count for the given data product ID
     * \param dataProductID data product ID for which message count is returned
     * \return message count
     */
    GRAVITY_API int getMessageCount(std::string dataProductID);

    /**
     * Method to return the byte count for the given data product ID
     * \param dataProductID data product ID for which byte count is returned
     * \return byte count
     */
    GRAVITY_API int getByteCount(std::string dataProductID);

    /**
     * Method to return the sample period start time
     * \return sample period start time (microsecond epoch time)
     */
    GRAVITY_API uint64_t getStartTime();

    /**
     * Method to return the sample period end time
     * \return sample period end time (microsecond epoch time)
     */
    GRAVITY_API uint64_t getEndTime();

    /**
     * Method to indicate that the collection period is complete
     */
    GRAVITY_API void done();

    /**
     * Method to return the sample period in seconds
     * \return sample period sec
     */
    GRAVITY_API double getSamplePeriodSeconds();

    /**
     * Method to send this MetricsData object on a zmq socket
     * \param socket zmq_socket over which to send this MetricsData object
     */
    GRAVITY_API void sendAsMessage(void* socket);

    /**
     * Method to populate this MetricsData object from zmq socket
     * \param socket zmq_socket from which to populate this MetricsData object
     */
    GRAVITY_API void populateFromMessage(void* socket);

    /**
     * Method to get the list of data product ID for which this
     * MetricsData has metrics
     * \return vector of data product IDs
     */
    GRAVITY_API std::vector<std::string> getDataProductIDs();
};

} /* namespace gravity */
#endif /* GRAVITYMETRICS_H_ */
