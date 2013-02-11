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
#include <tr1/memory>
#else
#include <memory>
#endif
#include <zmq.h>
#include <vector>
#include <map>
#include <string>
#include "protobuf/GravityMetricsDataPB.pb.h"

namespace gravity
{

using namespace std;
using namespace std::tr1;

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
	vector<zmq_pollitem_t> pollItems;

	void ready();
	void publishMetricsReport();

	bool metricsEnabled;
	int samplePeriod;
	int samplesPerPublish;
	string componentID;
	string ipAddr;
	map<std::pair<std::string,GravityMetricsPB_MessageType>,  GravityMetricsPB> metricsData;

	void collectMetrics(void* socket, GravityMetricsPB_MessageType type);
	void publishMetrics();
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
