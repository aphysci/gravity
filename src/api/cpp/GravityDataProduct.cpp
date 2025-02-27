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

namespace gravity
{

using namespace std;

GravityDataProduct::GravityDataProduct(string dataProductID) : gravityDataProductPB(new GravityDataProductPB())
{
    gravityDataProductPB->set_dataproductid(dataProductID);
}

GravityDataProduct::GravityDataProduct(const void* arrayPtr, int size)
    : gravityDataProductPB(new GravityDataProductPB())
{
    gravityDataProductPB->ParseFromArray(arrayPtr, size);
}

GravityDataProduct::~GravityDataProduct() {}

std::string GravityDataProduct::getDataProductID() const { return gravityDataProductPB->dataproductid(); }

void GravityDataProduct::setSoftwareVersion(string softwareVersion)
{
    gravityDataProductPB->set_softwareversion(softwareVersion);
}

std::string GravityDataProduct::getSoftwareVersion() const { return gravityDataProductPB->softwareversion(); }

uint64_t GravityDataProduct::getReceivedTimestamp() const
{
    uint64_t receivedTimestamp = 0;
    if (gravityDataProductPB->has_received_timestamp())
    {
        receivedTimestamp = gravityDataProductPB->received_timestamp();
    }
    return receivedTimestamp;
}

uint64_t GravityDataProduct::getGravityTimestamp() const { return gravityDataProductPB->timestamp(); }

void GravityDataProduct::setData(const void* data, int size)
{
    delete gravityDataProductPB->release_data();  //Looking at the protobuf, this seems necessary.
    gravityDataProductPB->set_data(data, size);
}

void GravityDataProduct::setData(const google::protobuf::Message& data)
{
    char* vdata = (char*)malloc(data.ByteSizeLong());
    data.SerializeToArray(vdata, data.ByteSizeLong());
    delete gravityDataProductPB->release_data();  //Looking at the protobuf, this seems necessary.
    gravityDataProductPB->set_data(vdata, data.ByteSizeLong());
    free(vdata);
    // Also implicitly set the message protocol and data_type.
    gravityDataProductPB->set_protocol("protobuf2");
    gravityDataProductPB->set_type_name(data.GetTypeName());
}

bool GravityDataProduct::getData(void* data, int size) const
{
    memcpy(data, gravityDataProductPB->data().c_str(), size);

    return true;
}

int GravityDataProduct::getDataSize() const { return gravityDataProductPB->data().length(); }

bool GravityDataProduct::populateMessage(google::protobuf::Message& data) const
{
    return data.ParseFromArray((void*)gravityDataProductPB->data().c_str(), getDataSize());
}

int GravityDataProduct::getSize() const { return gravityDataProductPB->ByteSizeLong(); }

void GravityDataProduct::parseFromArray(const void* arrayPtr, int size)
{
    gravityDataProductPB->ParseFromArray(arrayPtr, size);
}

bool GravityDataProduct::serializeToArray(void* arrayPtr) const
{
    return gravityDataProductPB->SerializeToArray(arrayPtr, gravityDataProductPB->ByteSizeLong());
}

bool GravityDataProduct::operator==(const GravityDataProduct& gdp) const
{
    // fastest test first...
    if (getDataSize() != gdp.getDataSize()) return false;
    if (getDataProductID().compare(gdp.getDataProductID()) != 0) return false;
    return memcmp(gravityDataProductPB->data().c_str(), gdp.gravityDataProductPB->data().c_str(), getDataSize()) == 0;
}

bool GravityDataProduct::operator!=(const GravityDataProduct& gdp) const { return !operator==(gdp); }

std::string GravityDataProduct::getComponentId() const { return gravityDataProductPB->componentid(); }

std::string GravityDataProduct::getDomain() const { return gravityDataProductPB->domain(); }

bool GravityDataProduct::isFutureResponse() const { return gravityDataProductPB->future_response(); }

bool GravityDataProduct::isCachedDataproduct() const
{
    return (gravityDataProductPB->has_is_cached_dataproduct() && gravityDataProductPB->is_cached_dataproduct());
}

void GravityDataProduct::setIsCachedDataproduct(bool cached)
{
    gravityDataProductPB->set_is_cached_dataproduct(cached);
}
std::string GravityDataProduct::getFutureSocketUrl() const { return gravityDataProductPB->future_socket_url(); }

bool GravityDataProduct::isRelayedDataproduct() const
{
    return (gravityDataProductPB->has_is_relayed_dataproduct() && gravityDataProductPB->is_relayed_dataproduct());
}

void GravityDataProduct::setIsRelayedDataproduct(bool relayed)
{
    gravityDataProductPB->set_is_relayed_dataproduct(relayed);
}

void GravityDataProduct::setProtocol(const std::string& protocol) { gravityDataProductPB->set_protocol(protocol); }

const std::string& GravityDataProduct::getProtocol() const { return gravityDataProductPB->protocol(); }

void GravityDataProduct::setTypeName(const std::string& dataType) { gravityDataProductPB->set_type_name(dataType); }

const std::string& GravityDataProduct::getTypeName() const { return gravityDataProductPB->type_name(); }

uint32_t GravityDataProduct::getRegistrationTime() const
{
    uint64_t registrationTime = 0;
    if (gravityDataProductPB->has_registration_time())
    {
        registrationTime = gravityDataProductPB->registration_time();
    }
    return registrationTime;
}

} /* namespace gravity */
