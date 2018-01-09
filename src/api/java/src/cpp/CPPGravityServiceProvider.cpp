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

#include "CPPGravityServiceProvider.h"
#include <iostream>

using namespace gravity;
using namespace std::tr1;

CPPGravityServiceProvider::~CPPGravityServiceProvider()
{}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
    unsigned char* array = new unsigned char[dataProduct.getSize()];
    dataProduct.serializeToArray(array);
    shared_ptr<GravityDataProduct> ret = request(serviceID, (char*)array, dataProduct.getSize());
	delete array;
	return ret;
}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(const std::string serviceID, char* array, int length)
{
    shared_ptr<GravityDataProduct> ret(new GravityDataProduct("CPP RESPONSE"));
    return ret;
}

