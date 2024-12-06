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

int main()
{
    GravityNode gn;
    gn.init("ConfigTest");

    cout << "LocalLogLevel=" << gn.getStringParam("LocalLogLevel") << endl;
    cout << "NetLogLevel=" << gn.getStringParam("NetLogLevel") << endl;

    cout << "Override=" << gn.getStringParam("Override") << endl;
    cout << "NonOverride=" << gn.getStringParam("NonOverride") << endl;

    cout << "Param1=" << gn.getStringParam("Param1") << endl;

    return 0;
}
