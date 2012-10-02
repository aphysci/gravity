//============================================================================
// Name        : HeartBeatDummy.cpp
// Author      : Tim Ludwinski
// Version     :
// Copyright   : APS 2012
// Description : Dummy program listening for a heartbeat
//============================================================================

#include <iostream>
#include <GravityNode.h>

#ifdef WIN32
#include <Windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#endif

using namespace std;
using namespace gravity;

class MyHeartbeatListener : public GravityHeartbeatListener {
	virtual void MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status);
};

void MyHeartbeatListener::MissedHeartbeat(std::string dataProductID, int microsecond_to_last_heartbeat, std::string status)
{
	cout << "Dataproduct: " << dataProductID << " missed heartbeat.  Last Heard: " << microsecond_to_last_heartbeat / 1000 << "ms.  Status: " << status << endl;
}

int main() {
	GravityNode gn;
	gn.init("HBListener");

	MyHeartbeatListener listener;
	gn.registerHeartbeatListener("hbdummydpid", 490000, listener);

	gn.waitForExit();

	return 0;
}
