//============================================================================
// Name        : HeartBeatDummy.cpp
// Author      : Tim Ludwinski
// Version     :
// Copyright   : APS 2012
// Description : Dummy program using the heartbeat
//============================================================================

#include <iostream>
#include <GravityNode.h>

using namespace std;
using namespace gravity;

int main() {
	GravityNode gn;
	gn.init("hbdummydpid");

	cout << "Start" << endl;	
	
	gn.startHeartbeat(490000); //About 1/2 second.

	cout << "Started" << endl;

	gn.waitForExit();
		
	return 0;
}