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
 * GravityDataProduct.cpp
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#include "GravityDataProduct.h"

namespace gravity {

using namespace std;

GravityDataProduct::GravityDataProduct(string dataProductID) : gravityDataProductPB(new GravityDataProductPB())
{
    gravityDataProductPB->set_dataproductid(dataProductID);
}

GravityDataProduct::GravityDataProduct(void* arrayPtr, int size) : gravityDataProductPB(new GravityDataProductPB())
{
    gravityDataProductPB->ParseFromArray(arrayPtr, size);
}

GravityDataProduct::~GravityDataProduct() {}

std::string GravityDataProduct::getDataProductID() const
{
    return gravityDataProductPB->dataproductid();
}

void GravityDataProduct::setSoftwareVersion(string softwareVersion)
{
    gravityDataProductPB->set_softwareversion(softwareVersion);
}

std::string GravityDataProduct::getSoftwareVersion() const
{
    return gravityDataProductPB->softwareversion();
}

uint64_t GravityDataProduct::getGravityTimestamp() const
{
    return gravityDataProductPB->timestamp();
}

void GravityDataProduct::setData(const void* data, int size)
{
    delete gravityDataProductPB->release_data(); //Looking at the protobuf, this seems necessary.
    gravityDataProductPB->set_data(data, size);
}

void GravityDataProduct::setData(const google::protobuf::Message& data)
{
    char* vdata = (char*)malloc(data.ByteSize());
    data.SerializeToArray(vdata, data.ByteSize());
    delete gravityDataProductPB->release_data(); //Looking at the protobuf, this seems necessary.
    gravityDataProductPB->set_data(vdata, data.ByteSize());
    free(vdata);
}

bool GravityDataProduct::getData(void* data, int size) const
{
    memcpy(data, gravityDataProductPB->data().c_str(), size);

    return true;
}

int GravityDataProduct::getDataSize() const
{
    return gravityDataProductPB->data().length();
}

bool GravityDataProduct::populateMessage(google::protobuf::Message& data) const
{
    return data.ParseFromArray((void*)gravityDataProductPB->data().c_str(), getDataSize());
}

int GravityDataProduct::getSize() const
{
    return gravityDataProductPB->ByteSize();
}

void GravityDataProduct::parseFromArray(void* arrayPtr, int size)
{
    gravityDataProductPB->ParseFromArray(arrayPtr, size);
}

bool GravityDataProduct::serializeToArray(void* arrayPtr) const
{
    return gravityDataProductPB->SerializeToArray(arrayPtr, gravityDataProductPB->ByteSize());
}

bool GravityDataProduct::operator==(const GravityDataProduct &gdp)
{
    // fastest test first...
    if (getDataSize() != gdp.getDataSize())
        return false;
    if (getDataProductID().compare(gdp.getDataProductID()) != 0)
        return false;
    return memcmp(gravityDataProductPB->data().c_str(), gdp.gravityDataProductPB->data().c_str(), getDataSize()) == 0;
}

std::string GravityDataProduct::getComponentId()
{
	return gravityDataProductPB->componentid();
}

std::string GravityDataProduct::getDomain()
{
	return gravityDataProductPB->domain();
}

bool GravityDataProduct::isFutureResponse()
{
	return gravityDataProductPB->future_response();
}

std::string GravityDataProduct::getFutureSocketUrl()
{
	return gravityDataProductPB->future_socket_url();
}

} /* namespace gravity */
