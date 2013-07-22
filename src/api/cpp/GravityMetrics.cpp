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
 * GravityMetrics.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: Chris Brundick
 */

#include "GravityMetrics.h"
#include "CommUtil.h"
#include <zmq.h>
#include <iostream>

using namespace std;

namespace gravity {

GravityMetrics::GravityMetrics() : startTime(0), endTime(0) {}

GravityMetrics::GravityMetrics(void* socket) 
{
    populateFromMessage(socket);
}

GravityMetrics::~GravityMetrics() {}

void GravityMetrics::incrementMessageCount(string dataProductID, int count)
{
    metrics[dataProductID].messageCount += count;
}

void GravityMetrics::incrementByteCount(string dataProductID, int count)
{
    metrics[dataProductID].byteCount += count;
}

void GravityMetrics::reset()
{
    map<string, MetricsSample>::iterator it;
    for (it = metrics.begin(); it != metrics.end(); ++it)
    {
        it->second.messageCount = 0;
        it->second.byteCount = 0;
    }
    startTime = gravity::getCurrentTime();
    endTime = 0;
}

void GravityMetrics::clear()
{
    metrics.clear();
    startTime = 0;
    endTime = 0;
}

void GravityMetrics::remove(std::string dataProductID)
{
    metrics.erase(dataProductID);
}

int GravityMetrics::getMessageCount(string dataProductID)
{
    int count = -1;
    if (metrics.count(dataProductID))
    {
    	count = metrics[dataProductID].messageCount;
    }
    return count;
}

int GravityMetrics::getByteCount(string dataProductID)
{
    int count = -1;
    if (metrics.count(dataProductID))
    {
    	count = metrics[dataProductID].byteCount;
    }
    return count;
}

uint64_t GravityMetrics::getStartTime()
{
    return startTime;
}

uint64_t GravityMetrics::getEndTime()
{
    return endTime;
}

void GravityMetrics::done()
{
    endTime = gravity::getCurrentTime();
}

double GravityMetrics::getSamplePeriodSeconds()
{
    double ret = -1;
    if (startTime > 0 && endTime > 0)
    {
	// convert from us to s
        ret = ((double)(endTime - startTime)) / 1e6;
    }
    return ret;
}

void GravityMetrics::sendAsMessage(void* socket)
{
    if (metrics.empty())
    {
        sendIntMessage(socket, 0, ZMQ_DONTWAIT);
    }
    else
    {
        sendIntMessage(socket, metrics.size(), ZMQ_SNDMORE);
        map<string, MetricsSample>::iterator it;
        for (it = metrics.begin(); it != metrics.end(); ++it)
        {
            sendStringMessage(socket, it->first, ZMQ_SNDMORE);
            sendIntMessage(socket, it->second.messageCount, ZMQ_SNDMORE);
            sendIntMessage(socket, it->second.byteCount, ZMQ_SNDMORE);
        }
	sendUint64Message(socket, startTime, ZMQ_SNDMORE);
	sendUint64Message(socket, endTime, ZMQ_DONTWAIT);
    }
}

void GravityMetrics::populateFromMessage(void* socket)
{
    clear();
    int size = readIntMessage(socket);
    if (size > 0)
    {
        for (int i = 0; i < size; i++)
        {
            std::string dataProductID = readStringMessage(socket);
            metrics[dataProductID].messageCount = readIntMessage(socket);
            metrics[dataProductID].byteCount = readIntMessage(socket);
        }
        startTime = readUint64Message(socket);
        endTime = readUint64Message(socket);
    }
}

vector<std::string> GravityMetrics::getDataProductIDs()
{
    vector<std::string> dataProductIDs;
    for (map<string, MetricsSample>::iterator it = metrics.begin(); it != metrics.end(); ++it)
    {
        dataProductIDs.push_back(it->first);
    }
    return dataProductIDs;
}

} /* namespace gravity */
