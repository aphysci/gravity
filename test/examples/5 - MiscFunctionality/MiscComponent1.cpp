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


int main()
{

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("MiscGravityComponentID1");

	//Get a parameter from either the gravity.ini config file, the MiscGravityComponentID.ini config file, or the config service.  
	int interval = gn.getIntParam("HeartbeatInterval", 500000);
	
	//Start a heartbeat that other components can listen to, telling them we're alive.  
	gn.startHeartbeat(interval);

	//Register a data product that is very fast only for components on this same machine.  
	gn.registerDataProduct("IPCDataProduct", 54531, "ipc");
	
	while(true)
	{
		//Publish the inter process communication data product.  
		GravityDataProduct ipcDataProduct("IPCDataProduct");
		
		std::string data = "hey!";
		ipcDataProduct.setData(data, data.length());
		
		gn.publish(ipcDataProduct);
	
		gravity::sleep(1000);
	}
}