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

#include <iostream>
#include <GravityNode.h>
#include "GravityLogRecorder.h"

int main(int argc, const char** argv)
{
    using namespace gravity;
    GravityNode gn;
    if (argc > 1) 
    {
        gn.init("GravityLogRecorder", std::string(argv[1]));
    }
    else 
    {
        gn.init("GravityLogRecorder");
    }

    LogRecorder lr(&gn, "MyBase");

    lr.start();

    gn.waitForExit();
}
