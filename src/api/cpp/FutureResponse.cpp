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

/*
 * FutureResponse.cpp
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#include "FutureResponse.h"

using namespace std;

namespace gravity {

FutureResponse::FutureResponse(string url) : GravityDataProduct("FutureResponse")
{	
	// Populate protobuf with future response specific fields
	gravityDataProductPB->set_future_response(true);
	gravityDataProductPB->set_future_socket_url(url);
}

FutureResponse::FutureResponse(const void* arrayPtr, int size) : GravityDataProduct(arrayPtr, size) {}

FutureResponse::~FutureResponse() {}

string FutureResponse::getUrl() const
{
	return gravityDataProductPB->future_socket_url();
}

void FutureResponse::setResponse(const GravityDataProduct& response)
{
	int size = response.getSize();
	char* data = new char[size];
	response.serializeToArray(data);
	setData(data, size);
	delete [] data;
}

} /* namespace gravity */
