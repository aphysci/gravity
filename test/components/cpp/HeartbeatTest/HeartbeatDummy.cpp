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
