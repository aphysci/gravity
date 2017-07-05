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



#include "CPPGravityRequestor.h"
#include <iostream>

using namespace gravity;

CPPGravityRequestor::~CPPGravityRequestor()
{}

void CPPGravityRequestor::requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response)
{
    unsigned char* array = new unsigned char[response.getSize()];
    response.serializeToArray(array);
    requestFilled(serviceID, requestID, (char*)array, response.getSize());
	delete[] array;
}

char CPPGravityRequestor::requestFilled(const std::string& serviceID, const std::string& requestID, char* array, int length)
{
    return 0;
}

