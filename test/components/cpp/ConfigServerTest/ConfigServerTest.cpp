//============================================================================
// Name        : ConfigServerTest.cpp
// Author      : Tim Ludwinski
// Version     :
// Copyright   : APS 2012
// Description : Dummy program using the Config Server
//============================================================================

#include <iostream>
#include <GravityNode.h>
#include <GravityConfigParser.h>

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

	GravityConfigParser gcp("config.ini"); //Not Using INI file.  Just here because we need something here.  
	gcp.ParseConfigService(gn, "dummycomponentid");

	cout << "LocalLogLevel: " << Log::LogLevelToString(gcp.getLocalLogLevel()) << endl;
	cout << "NetLogLevel: " << Log::LogLevelToString(gcp.getNetLogLevel()) << endl;

	return 0;
}