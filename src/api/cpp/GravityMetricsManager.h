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
 * GravityMetricsManager.h
 *
 *  Created on: Feb 3, 2013
 *      Author: Chris Brundick
 */

#ifndef GRAVITYMETRICSMANAGER_H_
#define GRAVITYMETRICSMANAGER_H_

#include "Utility.h"

#ifdef __GNUC__
#include <memory>
#else
#include <memory>
#endif
#include <zmq.h>
#include <vector>
#include <map>
#include <string>
#include "protobuf/GravityMetricsDataPB.pb.h"
#include "spdlog/spdlog.h"

namespace gravity
{

/**
 * The GravityMetricsManager is a component used internally by the GravityNode to
 * track messages and message size sent and received
 */
class GravityMetricsManager
{
private:
	void* context;
	void* metricsControlSocket;
	void* pubMetricsSocket;
	void* subMetricsSocket;
	void* metricsPubSocket;
	std::vector<zmq_pollitem_t> pollItems;

	void ready();
	void publishMetricsReport();

	bool metricsEnabled;
	int samplePeriod;
	int samplesPerPublish;
	std::string componentID;
	std::string ipAddr;
	std::map<std::pair<std::string,GravityMetricsPB_MessageType>,  GravityMetricsPB> metricsData;

	void collectMetrics(void* socket, GravityMetricsPB_MessageType type);
	void publishMetrics();
	
	std::shared_ptr<spdlog::logger> logger;
	
public:
	/**
	 * Constructor GravityMetricsManager
	 * \param context The zmq context in which the inproc socket will be established with the GravityNode
	 */
	GravityMetricsManager(void* context);

	/**
	 * Default destructor
	 */
	virtual ~GravityMetricsManager();

	/**
	 * Starts the GravityMetricsManager which will run forever
	 * Should be executed from GravityNode in its own thread with a shared zmq context.
	 */
	void start();
};

} /* namespace gravity */
#endif /* GRAVITYMETRICSMANAGER_H_ */
