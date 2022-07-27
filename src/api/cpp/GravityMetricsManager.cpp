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
* GravityMetricsManager.cpp
*
*  Created on: Feb 3, 2012
*      Author: Chris Brundick
*/

#include "GravityMetricsManager.h"
#include "GravityLogger.h"
#include "CommUtil.h"
#include "GravityMetricsUtil.h"
#include "GravityMetrics.h"
#include "zmq.h"
#include <sstream>
#include <iostream>

namespace gravity
{

    using namespace std;

    GravityMetricsManager::GravityMetricsManager(void* context)
    {
        // This is the zmq context that is shared with the GravityNode. Must use
        // a shared context to establish an inproc socket.
        this->context = context;
		logger = spdlog::get("GravityLogger");
    }

    GravityMetricsManager::~GravityMetricsManager() {}

    void GravityMetricsManager::start()
    {
        metricsPubSocket = zmq_socket(context, ZMQ_PUB);
        zmq_bind(metricsPubSocket, GRAVITY_METRICS_PUB);

        // Setup inproc socket to subscribe to metrics control messages
        metricsControlSocket = zmq_socket(context, ZMQ_SUB);
        zmq_connect(metricsControlSocket, GRAVITY_METRICS_CONTROL);
        zmq_setsockopt(metricsControlSocket, ZMQ_SUBSCRIBE, NULL, 0);

        // Setup comms channel to request metrics from the GravityPublishManager
        pubMetricsSocket = zmq_socket(context, ZMQ_REQ);
        int ret = zmq_connect(pubMetricsSocket, GRAVITY_PUB_METRICS_REQ);
        while (ret == -1)
        {
            sleep(1000);
            ret = zmq_connect(pubMetricsSocket, GRAVITY_PUB_METRICS_REQ);
        }

        // Setup comms channel to request metrics from the GravitySubscriptionManager
        subMetricsSocket = zmq_socket(context, ZMQ_REQ);
        ret = zmq_connect(subMetricsSocket, GRAVITY_SUB_METRICS_REQ);
        while (ret == -1)
        {
            sleep(1000);
            ret = zmq_connect(subMetricsSocket, GRAVITY_SUB_METRICS_REQ);
        }

        // Setup the poll item for control
        zmq_pollitem_t pollItemControl;
        pollItemControl.socket = metricsControlSocket;
        pollItemControl.events = ZMQ_POLLIN;
        pollItemControl.fd = 0;
        pollItemControl.revents = 0;
        pollItems.push_back(pollItemControl);

        // Configured. Signal our readiness
        ready();

        int pollFlag;
        int sampleCount = 0;
        metricsEnabled = false;
        // Process forever...
        while (true)
        {
            pollFlag = metricsEnabled ? 0 : -1;
            // Start polling metrics control socket
            int rc = zmq_poll(&pollItems[0], pollItems.size(), pollFlag); // 0 --> return immediately, -1 --> blocks
            if (rc == -1)
            {
                // Interrupted
                break;
            }

            // Process incoming data requests from the gravity node
            if (pollItems[0].revents & ZMQ_POLLIN)
            {
                // Get new GravityNode request
                string command = readStringMessage(metricsControlSocket);

                if (command == "MetricsEnable")
                {
                    metricsEnabled = true;

                    samplePeriod = readIntMessage(metricsControlSocket);
                    samplesPerPublish = readIntMessage(metricsControlSocket);
                    sampleCount = 0;

                    componentID = readStringMessage(metricsControlSocket);
                    ipAddr = readStringMessage(metricsControlSocket);
                    regTimestamp = readIntMessage(metricsControlSocket);

                    // Send metrics enable message to the collectors
                    sendStringMessage(pubMetricsSocket, command, ZMQ_DONTWAIT);
                    string s = readStringMessage(pubMetricsSocket);
                    sendStringMessage(subMetricsSocket, command, ZMQ_DONTWAIT);
                    s = readStringMessage(subMetricsSocket);
                }
                else if (command == "MetricsDisable")
                {
                    metricsEnabled = false;
                }
                else if (command == "kill")
                {
                    break;
                }
                else
                {
					logger->warn("GravityMetricsManager received unknown command '{}' from GravityNode", command);
                }
            }

            if (metricsEnabled)
            {
                // Wait for samplePeriod seconds and make metrics request
                gravity::sleep(samplePeriod * 1000);
                collectMetrics(pubMetricsSocket, GravityMetricsPB::PUBLICATION);
                collectMetrics(subMetricsSocket, GravityMetricsPB::SUBSCRIPTION);

                // If we've collected samplesPerPublish samples, publish metrics
                if (++sampleCount == samplesPerPublish)
                {
                    publishMetrics();
                    sampleCount = 0;
                }
            }
        }

        // Clean up sockets
        zmq_close(metricsPubSocket);
        zmq_close(metricsControlSocket);
        zmq_close(pubMetricsSocket);
        zmq_close(subMetricsSocket);
    }

    void GravityMetricsManager::ready()
    {
        // Create the request socket
        void* initSocket = zmq_socket(context, ZMQ_REQ);

        // Connect to service
        zmq_connect(initSocket, "inproc://gravity_init");

        // Send request to service provider
        sendStringMessage(initSocket, "GravityMetricsManager", ZMQ_DONTWAIT);

        zmq_close(initSocket);
    }

    void GravityMetricsManager::collectMetrics(void* socket, GravityMetricsPB_MessageType type)
    {
        GravityMetrics metrics;
        sendStringMessage(socket, "GetMetrics", ZMQ_DONTWAIT);
        metrics.populateFromMessage(socket);

        vector<string> dataProductIDs = metrics.getDataProductIDs();
        for (vector<string>::iterator it = dataProductIDs.begin(); it != dataProductIDs.end(); ++it)
        {
            string dataProductID = *it;

            pair<string,GravityMetricsPB_MessageType> key (dataProductID, type);
            GravityMetricsPB gmPB = metricsData[key];
            if (!gmPB.has_dataproductid())
            {
                gmPB.set_dataproductid(dataProductID);
                gmPB.set_messagetype(type);
            }
            gmPB.add_numbytes(metrics.getByteCount(dataProductID));
            gmPB.add_nummessages(metrics.getMessageCount(dataProductID));
            gmPB.add_starttime(metrics.getStartTime());
            gmPB.add_endtime(metrics.getEndTime());
            metricsData[key] = gmPB;
        }
    }

    void GravityMetricsManager::publishMetrics()
    {
        GravityMetricsDataPB metrics;
        metrics.set_componentid(componentID);
        metrics.set_ipaddress(ipAddr);

        map<pair<string, GravityMetricsPB_MessageType>, GravityMetricsPB>::iterator it;
        for (it = metricsData.begin(); it != metricsData.end(); ++it)
        {
            GravityMetricsPB* gmPB = metrics.add_metrics();
            *gmPB = it->second;
        }

        GravityDataProduct gdp(GRAVITY_METRICS_DATA_PRODUCT_ID);
        gdp.setTimestamp(gravity::getCurrentTime());
        gdp.setRegistrationTime(regTimestamp);
        gdp.setData(metrics);

        // Send publish command and empty filter
        sendStringMessage(metricsPubSocket, "publish", ZMQ_SNDMORE);
        sendStringMessage(metricsPubSocket, gdp.getDataProductID(), ZMQ_SNDMORE);
        sendUint64Message(metricsPubSocket, gdp.getGravityTimestamp(), ZMQ_SNDMORE);
        sendStringMessage(metricsPubSocket, componentID, ZMQ_SNDMORE);

        // Publish metrics
        sendGravityDataProduct(metricsPubSocket, gdp, ZMQ_DONTWAIT);

        // Clear metrics data for next round
        metricsData.clear();
    }

} /* namespace gravity */
