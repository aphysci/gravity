//============================================================================
// Name        : HeartBeatDummy.cpp
// Author      : Tim Ludwinski
// Version     :
// Copyright   : APS 2012
// Description : Dummy program using the heartbeat
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

int main() {
	GravityNode gn;
	gn.init();

	cout << "Start" << endl;	
	
	gn.startHeartbeat("hbdummydpid", 490000); //About 1/2 second.

	cout << "Started" << endl;
	
	while(true)
		sleep(2000000000);

	cout << "Hey" << endl;
		
	return 0;
}