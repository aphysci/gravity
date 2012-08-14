/*
 * ServiceDirectory.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: Mark Barger
 */

#include "ServiceDirectory.h"
#include "DataProductDescriptionPB.pb.h"
#include "ComponentLookupRequestPB.pb.h"
#include "ComponentLookupResponsePB.pb.h"
#include "zmq.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

int main(void)
{
	gravity::ServiceDirectory* serviceDirectory = new gravity::ServiceDirectory();
	serviceDirectory->start();
}

namespace gravity
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
		string* requestType = new string((char*)zmq_msg_data(&envelope), size);
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
			ComponentLookupRequestPB lookupRequest;
			lookupRequest.ParseFromArray(data, size);
			string url = registrationMap[lookupRequest.dataproductid()];

			ComponentLookupResponsePB lookupResponse;
			lookupResponse.set_url(url);

			// Send reply back to client
			zmq_msg_init_size(&response, lookupResponse.ByteSize());
			lookupResponse.SerializeToArray(zmq_msg_data(&response), lookupResponse.ByteSize());
			zmq_sendmsg(socket, &response, 0);
			zmq_msg_close(&response);
		}
		else if (strcmp(requestType->c_str(), "register"))
		{
			DataProductDescriptionPB dpDescription;
			dpDescription.ParseFromArray(data, size);
			string url = registrationMap[dpDescription.dataproductid()];
			string ack = "ack";
			if (url == "")
				registrationMap[dpDescription.dataproductid()] = dpDescription.url();
			else
			{
				ack = "error";
				cout << "DataProductID " << dpDescription.dataproductid() << " is already registered." << endl;
			}

			// Send reply back to client
			zmq_msg_init_size(&response, ack.length());
			memcpy(zmq_msg_data(&response), ack.c_str(), ack.length());
			zmq_sendmsg(socket, &response, 0);
			zmq_msg_close(&response);
		}
	}
}

} /* namespace esmf */
