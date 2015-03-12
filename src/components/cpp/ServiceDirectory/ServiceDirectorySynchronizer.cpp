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
 * ServiceDirectorySynchronzier.cpp
 *
 *  Created on: Jan 8, 2015
 *      Author: Chris Brundick
 */

#include "ServiceDirectorySynchronizer.h"
#include "GravityLogger.h"
#include "CommUtil.h"

#include "protobuf/ComponentDataLookupResponsePB.pb.h"
#include "protobuf/ComponentLookupRequestPB.pb.h"
#include "protobuf/ServiceDirectoryResponsePB.pb.h"

#include <iostream>

using namespace std;

namespace gravity
{

ServiceDirectorySynchronizer::ServiceDirectorySynchronizer(void* context, string url)
{
	// This is the zmq context that is shared with the ServiceDirectory. Must use
    // a shared context to establish an inproc socket.
    this->context = context;
	this->ownURL = url;
}

ServiceDirectorySynchronizer::~ServiceDirectorySynchronizer() {};

void ServiceDirectorySynchronizer::start()
{
	// Setup inproc socket to subscribe to commands from Service Directory
    commandSocket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(commandSocket, "inproc://service_directory_synchronizer");
    zmq_setsockopt(commandSocket, ZMQ_SUBSCRIBE, NULL, 0);

	// Setup the poll item for control
    zmq_pollitem_t pollItemControl;
    pollItemControl.socket = commandSocket;
    pollItemControl.events = ZMQ_POLLIN;
    pollItemControl.fd = 0;
    pollItemControl.revents = 0;
    pollItems.push_back(pollItemControl);	

	// Create request socket for update to own Service Directory
	requestSocket = zmq_socket(context, ZMQ_REQ);

	// Create poll item for response to this request and add to vector
	// we're listening to
	zmq_pollitem_t pollItem;
	pollItem.socket = requestSocket;
	pollItem.events = ZMQ_POLLIN;
	pollItem.fd = 0;
	pollItem.revents = 0;
	pollItems.push_back(pollItem);

	// Connect to service
	zmq_connect(requestSocket, ownURL.c_str());

	pendingResponse = false;

	while (true)
    {
		// Start polling metrics control socket		
        int rc = zmq_poll(&pollItems[0], pollItems.size(), -1); // 0 --> return immediately, -1 --> blocks
        if (rc == -1)
        {
            // Interrupted
            break;
        }

        // Process incoming data requests from the gravity node
        if (pollItems[0].revents & ZMQ_POLLIN)
        {		
            // Get new GravityNode request
            string command = readStringMessage(commandSocket);		

			if (command == "Add")
			{
				// Get Domain from message 
				string domain = readStringMessage(commandSocket);

				// Get URL from message
				string domainIP = readStringMessage(commandSocket);

				Log::message("Received Domain Add command for %s:%s", domain.c_str(), domainIP.c_str());

				// We need to sychronize with a newly connected domain
				if (syncMap.find(domain) == syncMap.end())
				{					
					// Create details
					shared_ptr<SyncDomainDetails> details(new SyncDomainDetails());					
					details->domain = domain;
					details->ipAddress = domainIP;
					details->pollItemIndex = pollItems.size();
					details->initialized = false;

					// Create the request socket to the other Service Directory
					details->socket = zmq_socket(context, ZMQ_REQ);

					// Create poll item for response to this request and add to vector
					// we're listening to
					zmq_pollitem_t pollItem;
					pollItem.socket = details->socket;
					pollItem.events = ZMQ_POLLIN;
					pollItem.fd = 0;
					pollItem.revents = 0;
					pollItems.push_back(pollItem);

					// Connect to service
					zmq_connect(details->socket, domainIP.c_str());

					// Construct lookup request. This will register us as a 
					// subscriber to updates from the "remote" service directory
					GravityDataProduct request("ComponentLookupRequest");
					ComponentLookupRequestPB lookup;
					lookup.set_lookupid("ServiceDirectory_DomainDetails");
					lookup.set_type(gravity::ComponentLookupRequestPB_RegistrationType_DATA);
					lookup.set_domain_id(domain);
					request.setData(lookup);

					// Submit request
					zmq_msg_t data;
					zmq_msg_init_size(&data, request.getSize());
					request.serializeToArray(zmq_msg_data(&data));
					zmq_sendmsg(details->socket, &data, ZMQ_DONTWAIT);
					zmq_msg_close(&data);

					// Save details to out internal map
					syncMap[domain] = details;
				}
			}		
			else if (command == "Remove")
			{
				// Get Domain from message 
				string domain = readStringMessage(commandSocket);

				Log::message("Received Domain Remove command for '%s'", domain.c_str());

				//make sure we have a mapping to the domain
				if(syncMap.find(domain)!=syncMap.end())
				{
					// A domain is no longer available and needs to be removed. Get 
					// details from map
					shared_ptr<SyncDomainDetails> details = syncMap[domain];

					// Remove our subscription listener from pollItems vector
					// (i.e. no longer care to receive updates)
					pollItems.erase(pollItems.begin() + details->pollItemIndex);

					// Close SUB socket
					zmq_close(details->socket);				

					// Clear all domain data from Service Directory							
					for (int i = 0; i < details->providerMap.service_provider_size(); i++)
					{
						string productID = details->providerMap.service_provider(i).product_id();						
						for (int j = 0; j < details->providerMap.service_provider(i).url_size(); j++)
						{
							string url = details->providerMap.service_provider(i).url(j);						
							createUnregistrationRequest(productID, url, details->providerMap.change().registration_type());
						}
					}								
					for (int i = 0; i < details->providerMap.data_provider_size(); i++)
					{
						string productID = details->providerMap.data_provider(i).product_id();
						for (int j = 0; j < details->providerMap.data_provider(i).url_size(); j++)
						{
							string url = details->providerMap.data_provider(i).url(j);						
							createUnregistrationRequest(productID, url, details->providerMap.change().registration_type());
						}			
					}				

					// Remove from map
					syncMap.erase(domain);
				}
			}			
		}
		else if (pollItems[1].revents & ZMQ_POLLIN)
        {
			// This is a response from our request to update our own Service Directory
			Log::debug("Received response from own SD for an update");

			// Read (and ignore for now) the response
			zmq_msg_t message;
			zmq_msg_init(&message);
			zmq_recvmsg(pollItems[1].socket, &message, 0);
			zmq_msg_close(&message);

			// Clear the flag indicating that we're no longer awaiting a response and
			// can send another update
			pendingResponse = false;
		}

		if (pollItems.size() > 2)
		{
			// Now check for any updates from synchronized service directories
			vector<zmq_pollitem_t>::iterator pollItemIter = pollItems.begin() + 2;

			// Check for responses
			while(pollItemIter != pollItems.end())
			{
				if (pollItemIter->revents && ZMQ_POLLIN)
				{
					// Process response from ServiceDirectory
					zmq_msg_t message;
					zmq_msg_init(&message);
					zmq_recvmsg(pollItemIter->socket, &message, 0);

					// Create new GravityDataProduct from the incoming message
					GravityDataProduct response(zmq_msg_data(&message), zmq_msg_size(&message));

					if (response.getDataProductID() == "DataProductRegistrationResponse")
					{					
						// This is a response to our request for a subscription to
						// updates from the "remote" service directory
						ComponentDataLookupResponsePB resp;
						response.populateMessage(resp);
						string domain = resp.domain_id();
						string url = resp.url(0);

						Log::message("Received DataProductRegistrationResponse response from ServiceDirectory for domain: '%s'", domain.c_str());

						// Get the details for the domain that is providing an update
						shared_ptr<SyncDomainDetails> details = syncMap[domain];

						// Clean up the request socket (to be replaced with a SUB socket)
						zmq_close(details->socket);

						// Set up subscription socket for updates from the "remote" service directory
						details->socket = zmq_socket(context, ZMQ_SUB);
						details->initialized = false;

						// Update poll item with new subscription socket
						// (replace REP socket with new SUB socket)
						(*pollItemIter).socket = details->socket;
						(*pollItemIter).fd = 0;
						(*pollItemIter).revents = 0;					

						// Connect to publisher
						zmq_connect(details->socket, url.c_str());
						zmq_setsockopt(details->socket, ZMQ_SUBSCRIBE, NULL, 0);					
					}
					else if (response.getDataProductID() == "ServiceDirectory_DomainDetails")
					{
						// This is a subscription update of a service directory's content map
						ServiceDirectoryMapPB providerMap;
						response.populateMessage(providerMap);					
						string domain = providerMap.domain();

						Log::message("Received update from ServiceDirectory for domain: '%s'", domain.c_str());

						shared_ptr<SyncDomainDetails> details = syncMap[domain];
						details->providerMap.Clear();
						response.populateMessage(details->providerMap);					
						if (!details->initialized)
						{
							// First update from this Service Directory
							details->initialized = true;
                            Log::message("Initial Update from domain '%s'", domain.c_str());

							// Test code
							//printMap(providerMap);

							// Send all content as updates to our service directory
							for (int i = 0; i < providerMap.service_provider_size(); i++)
							{
								string productID = providerMap.service_provider(i).product_id();						
								for (int j = 0; j < providerMap.service_provider(i).url_size(); j++)
								{
									string url = providerMap.service_provider(i).url(j);
									string componentID = providerMap.service_provider(i).component_id(j);									
									createRegistrationRequest(productID, url, componentID, domain, providerMap.change().registration_type());
								}
							}								
							for (int i = 0; i < providerMap.data_provider_size(); i++)
							{
								string productID = providerMap.data_provider(i).product_id();
								for (int j = 0; j < providerMap.data_provider(i).url_size(); j++)
								{
									string url = providerMap.data_provider(i).url(j);
									string componentID = providerMap.data_provider(i).component_id(j);									
									createRegistrationRequest(productID, url, componentID, domain, providerMap.change().registration_type());
								}
							}
						}
						else
						{
							// Incremental update							
							string productID = providerMap.change().product_id();
							string url = providerMap.change().url();
							string change = providerMap.change().change_type() == ProductChange_ChangeType_ADD ? "Add" : "Remove";
							string type = providerMap.change().registration_type() == ProductChange_RegistrationType_DATA ? "Data" : "Service";	
							string componentID = providerMap.change().component_id();
							Log::message("Incremental Update from domain '%s': %s %s named %s at %s", domain.c_str(), 
											change.c_str(), type.c_str(), productID.c_str(), url.c_str());							

							if (providerMap.change().change_type() == ProductChange_ChangeType_ADD)
							{
								createRegistrationRequest(productID, url, componentID, domain, providerMap.change().registration_type());								
							}
							else
							{
								createUnregistrationRequest(productID, url, providerMap.change().registration_type());								
							}
						}
					}				

					// Clean up message
					zmq_msg_close(&message);				
				}
				pollItemIter++;
			}		
		}
		
		// If we're not awaiting a response from our SD for an update and we
		// have another one update to send then we'll send it
		if (!pendingResponse && !registrationUpdates.empty())
		{
			// Pop the next update to send from the queue
			shared_ptr<GravityDataProduct> gdp = registrationUpdates.front();
			registrationUpdates.pop();

			// Send data product
			zmq_msg_t data;
			zmq_msg_init_size(&data, gdp->getSize());
			gdp->serializeToArray(zmq_msg_data(&data));
			zmq_sendmsg(requestSocket, &data, ZMQ_DONTWAIT);
			zmq_msg_close(&data);

			// Set flag to indicate we're awaiting a response
			pendingResponse = true;
		}
	}
}

// Utility to create a RegistrationRequest message and add it to the queue to be submitted
void ServiceDirectorySynchronizer::createRegistrationRequest(string productID, string url, string componentID, 
															 string domain, ProductChange_RegistrationType type)
{
	ServiceDirectoryRegistrationPB_RegistrationType rtype = type == ProductChange_RegistrationType_DATA ?
			ServiceDirectoryRegistrationPB_RegistrationType_DATA : ServiceDirectoryRegistrationPB_RegistrationType_SERVICE;

	ServiceDirectoryRegistrationPB registerRequest;
	registerRequest.set_id(productID);
	registerRequest.set_url(url);
	registerRequest.set_type(rtype);	
	registerRequest.set_component_id(componentID);
	registerRequest.set_domain(domain);

	shared_ptr<GravityDataProduct> gdp(new GravityDataProduct("RegistrationRequest"));
	gdp->setData(registerRequest);
	registrationUpdates.push(gdp);
}

// Utility to create a UnregistrationRequest message and add it to the queue to be submitted
void ServiceDirectorySynchronizer::createUnregistrationRequest(string productID, string url, ProductChange_RegistrationType type)
{
	ServiceDirectoryUnregistrationPB_RegistrationType rtype = type == ProductChange_RegistrationType_DATA ?
				ServiceDirectoryUnregistrationPB_RegistrationType_DATA : ServiceDirectoryUnregistrationPB_RegistrationType_SERVICE;

	ServiceDirectoryUnregistrationPB unregisterRequest;
	unregisterRequest.set_id(productID);
	unregisterRequest.set_url(url);
	unregisterRequest.set_type(rtype);	

	shared_ptr<GravityDataProduct> gdp(new GravityDataProduct("UnregistrationRequest"));							
	gdp->setData(unregisterRequest);
	registrationUpdates.push(gdp);
}

// test function
void ServiceDirectorySynchronizer::printMap(ServiceDirectoryMapPB providerMap)
{
	cout << "Services:" << endl;
	for (int i = 0; i < providerMap.service_provider_size(); i++)
	{
		string providerID = providerMap.service_provider(i).product_id();
		cout << "   provider: " << providerID << endl;
		for (int j = 0; j < providerMap.service_provider(i).url_size(); j++)
		{
			string url = providerMap.service_provider(i).url(j);
			cout << "      url: " << url << endl;
			string componentID = providerMap.service_provider(i).component_id(j);
			cout << "      componentID: " << componentID << endl;
			string domainID = providerMap.service_provider(i).domain_id(j);
			cout << "      domainID: " << domainID << endl;
		}
	}			

	cout << "Data:" << endl;
	for (int i = 0; i < providerMap.data_provider_size(); i++)
	{
		string providerID = providerMap.data_provider(i).product_id();
		cout << "   provider: " << providerID << endl;
		for (int j = 0; j < providerMap.data_provider(i).url_size(); j++)
		{
			string url = providerMap.data_provider(i).url(j);
			cout << "      url: " << url << endl;
			string componentID = providerMap.data_provider(i).component_id(j);
			cout << "      componentID: " << componentID << endl;
			string domainID = providerMap.data_provider(i).domain_id(j);
			cout << "      domainID: " << domainID << endl;
		}
	}
	cout << endl;
}

}/*namespace gravity*/