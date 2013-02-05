#include <iostream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <Utility.h>

using namespace gravity;

int main()
{

	GravityNode gn;
	//Initialize gravity, giving this node a componentID.  
	gn.init("MiscGravityComponentID1");

	//Get a parameter from either the Gravity.ini config file, the MiscGravityComponentID.ini config file, or the config service.
	int interval = gn.getIntParam("HeartbeatInterval", 500000);
	
	//Start a heartbeat that other components can listen to, telling them we're alive.  
	gn.startHeartbeat(interval);

	// IPC isn't supported in Windows.
#ifndef WIN32
	//Register a data product that is very fast only for components on this same machine.  
	gn.registerDataProduct("IPCDataProduct", GravityTransportTypes::IPC);
#endif
	
	//Let this exit after a few seconds so the heartbeat listener in MiscComponent2 will be notified when it goes away.
	int count = 5;
	while(count-- > 0)
	{
#ifndef WIN32
		//Publish the inter process communication data product.  
		GravityDataProduct ipcDataProduct("IPCDataProduct");
		
		std::string data = "hey!";
		ipcDataProduct.setData(data.c_str(), data.length());
		
		gn.publish(ipcDataProduct);
#endif
		gravity::sleep(1000);
	}
}
