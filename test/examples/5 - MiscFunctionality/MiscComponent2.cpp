#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

using namespace gravity;

class MiscHBListener : public GravityHeartbeatListener
{
public:
	virtual void MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status);
}

void MiscHBListener::MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status)
{
	Log::warning("Missed Heartbeat.  Last heartbeat %d microseconds ago.  ", microsecond_to_last_heartbeat);
}

//Declare a class for receiving Published messages.  
class MiscGravitySubscriber : public GravitySubscriber
{
public:
	virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
};

void MiscGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
	//Get a raw message
	int size = dataProduct.getDataSize();
	char* message = new char[size];
	dataProduct.getData(message, size);
	
	//Output the message
	Log::message("Got message: %s", message);
	//Don't forget to free the memory we allocated.  
	delete message;
}

int main()
{
	using namespace gravity;

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("MiscGravityComponentID2");
	
	//Get a parameter from either the gravity.ini config file, the MiscGravityComponentID.ini config file, or the config service.  
	int interval = gn.getIntParam("HeartbeatInterval", //Param Name
									500000); //Default value.  
	
	//Get another parameter from gravity.  
	bool listen_for_heartbeat = gn.getBoolParam("HeartbeatListener", true);
	MiscHBListener hbl;
	
	if(listen_for_heartbeat)
		gn.registerHeartbeatListener("MiscGravityComponentID1", interval, hbl);

	//Subscribe to the IPC data product.  
	MiscGravitySubscriber ipcSubscriber;
	gn.subscribe("ipc://IPCDataProduct", "IPCDataProduct", ipcSubscriber);
	
	gn.waitForExit();
}