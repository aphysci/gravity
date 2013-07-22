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

#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

using namespace gravity;

class MiscHBListener : public GravityHeartbeatListener
{
public:
	virtual void MissedHeartbeat(std::string dataProductID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds);
	virtual void ReceivedHeartbeat(std::string dataProductID, int64_t& interval_in_microseconds);
};

void MiscHBListener::MissedHeartbeat(std::string dataProductID, int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds)
{
	Log::warning("Missed Heartbeat.  Last heartbeat %d microseconds ago.  ", microsecond_to_last_heartbeat);
}

void MiscHBListener::ReceivedHeartbeat(std::string dataProductID, int64_t& interval_in_microseconds)
{
	Log::warning("Received Heartbeat.");
	// Now that we've received one, change the interval to 10 seconds.
	interval_in_microseconds = 10 * 1000000;
}

//Declare a class for receiving Published messages.  
class MiscGravitySubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts);
};

void MiscGravitySubscriber::subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts)
{
	for(std::vector< shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin();
			i != dataProducts.end(); i++)
	{
		//Get a raw message
		int size = (*i)->getDataSize();
		char* message = new char[size+1];
		(*i)->getData(message, size);
		message[size] = 0; // null terminate
		
		//Output the message
		Log::message("Got message: %s", message);
		//Don't forget to free the memory we allocated.  
		delete message;
	}
}

int main()
{
	using namespace gravity;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("MiscGravityComponentID2");

	//Get a parameter from either the Gravity.ini config file, the MiscGravityComponentID.ini config file, or the config service.
	int interval = gn.getIntParam("HeartbeatInterval", //Param Name
									1000000); //Default value.
	
	//Get another parameter from gravity.  
	bool listen_for_heartbeat = gn.getBoolParam("HeartbeatListener", true);
	MiscHBListener hbl;
	
	if(listen_for_heartbeat)
		gn.registerHeartbeatListener("MiscGravityComponentID1", interval, hbl);

	// IPC isn't supported in Windows.
#ifndef WIN32
	//Subscribe to the IPC data product.  
	MiscGravitySubscriber ipcSubscriber;
	gn.subscribe("IPCDataProduct", ipcSubscriber);
#endif
	
	gn.waitForExit();
}
