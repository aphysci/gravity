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

#include <GravityNode.h>
#include <sstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  using namespace gravity;
  const char* myDataProductID = "dummydataproduct";

  if(argc > 1)
    myDataProductID = argv[1];

  GravityNode gn;
  gn.init("DummyPublisher");
  gn.registerDataProduct(myDataProductID, GravityTransportType::TCP);

  GravityDataProduct dataProduct(myDataProductID);
  int count = 0;

  while(true)
  {
    stringstream ss("");
    ss << count;
    string cnt_str = ss.str();

    //cout << cnt_str.length() << endl;
    cout << cnt_str << endl;
    
    dataProduct.setData((void*)cnt_str.c_str(), cnt_str.length());

    gn.publish(dataProduct);

    gravity::sleep(1000);

    count++;
    count = count % 20;
  }

  gn.unregisterDataProduct(myDataProductID);

}
