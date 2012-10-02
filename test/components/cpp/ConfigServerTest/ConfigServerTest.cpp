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

using namespace std;
using namespace gravity;

int main() {
	GravityNode gn;
	gn.init("ConfigTest");

	cout << "LocalLogLevel=" << gn.getStringParam("LocalLogLevel") << endl;
	cout << "NetLogLevel=" << gn.getStringParam("NetLogLevel") << endl;
	
	cout << "Override=" << gn.getStringParam("Override")  << endl;
	cout << "NonOverride=" << gn.getStringParam("NonOverride")  << endl;
	
	cout << "Param1=" << gn.getStringParam("Param1") << endl;

	return 0;
}