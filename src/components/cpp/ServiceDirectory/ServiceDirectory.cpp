/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: esmf
 */

#include "ServiceDirectory.h"
#include "zmq.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

#include <ComponentLookupRequest.h>
#include <ComponentLookupResponse.h>
#include <DataProductDescription.h>
#include <RadarData.h>
#include <ShipMotionData.h>

using namespace std;

int main(void)
{
	esmf::ServiceDirectory* serviceDirectory = new esmf::ServiceDirectory();
	serviceDirectory->start();
}

namespace esmf
{

ServiceDirectory::ServiceDirectory()
{
}

ServiceDirectory::~ServiceDirectory()
{
}

void ServiceDirectory::start()
{
	void *context = zmq_init(1);

	//   Socket to talk to clients
	void *socket = zmq_socket(context, ZMQ_REP);
	zmq_bind(socket, "tcp://*:5555");

	zmq_msg_t request, response, envelope;
	while (1)
	{
		//   Wait for next request
		zmq_msg_init(&envelope);
		cout << "Waiting for lookup request..." << flush;
		zmq_recvmsg(socket, &envelope, 0);
		int size = zmq_msg_size(&envelope);
		std::string* requestType = new std::string((char*)zmq_msg_data(&envelope), size);
		zmq_msg_close(&envelope);
		cout << "got a " << requestType << " request" << endl;

		zmq_msg_init(&request);
		cout << "Waiting for data..." << flush;
		zmq_recvmsg(socket, &request, 0);
		size = zmq_msg_size(&request);
		void* data = (char*)malloc(size + 1);
		memcpy(data, zmq_msg_data(&request), size);
		zmq_msg_close(&request);
		cout << "received" << endl;

		if (strcmp(requestType->c_str(), "lookup"))
		{
			std::string connectionString = "";
			ComponentLookupRequest* lookupRequest = new ComponentLookupRequest(data, size);
			vector<DataProductDescription> dataProducts = this->registrationMap[lookupRequest->getDataProductID()];

			ComponentLookupResponse* lookupResponse = new ComponentLookupResponse();
			for (unsigned int i = 0; i < dataProducts.size(); i++)
			{
				DataProductDescription desc = dataProducts[i];
				if (lookupRequest->getMinDataRate() <= desc.getDataRate() && lookupRequest->getMaxDataRate() >= desc.getDataRate())
				{
					lookupResponse->addDataProductDescription(desc);
				}
			}

			// Send reply back to client
			zmq_msg_init_size(&response, lookupResponse->getSize());
			lookupResponse->serializeToArray(zmq_msg_data(&response));
			zmq_sendmsg(socket, &response, 0);
			zmq_msg_close(&response);
		}
		else if (strcmp(requestType->c_str(), "register"))
		{
			DataProductDescription* dpDescription = new DataProductDescription(data, size);
			vector<DataProductDescription> dataProducts = this->registrationMap[dpDescription->getDataProductID()];
			dataProducts.push_back(*dpDescription);

			// Send reply back to client
			string ack = "ack";
			zmq_msg_init_size(&response, ack.length());
			memcpy(zmq_msg_data(&response), ack.c_str(), ack.length());
			zmq_sendmsg(socket, &response, 0);
			zmq_msg_close(&response);
		}
	}
}

} /* namespace esmf */
