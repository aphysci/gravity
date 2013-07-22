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

//class GravityNode {};
#include <iostream>
#include "GravityLogger.h"

int main()
{
  using namespace gravity;
  GravityNode gn;
  gn.init("TestLogger");

  cout << "test" << endl;

  //Log::setLocalLevel(Log::TRACE);
  //Log::init(gn, "test.log", 43211);

  getchar();

  cout << "finished init" << endl;

  Log::trace("Trace");

  Log::debug("1+1=%d", (1+1));

  //  OtherFunc();

  Log::message("Hi %s", "there");

  Log::warning("Warn!!!");

  Log::critical("1!=2");

  Log::fatal("AAAAAaaaaaaahhhhhh....");

}
