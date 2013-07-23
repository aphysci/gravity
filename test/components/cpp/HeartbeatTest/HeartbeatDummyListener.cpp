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
