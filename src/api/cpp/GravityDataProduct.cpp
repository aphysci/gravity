/*
 * GravityDataProduct.cpp
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#include "GravityDataProduct.h"

namespace gravity {

GravityDataProduct::GravityDataProduct(string dataProductID)
{
	gravityDataProductPB = new GravityDataProductPB();
	gravityDataProductPB->set_dataproductid(dataProductID);

	filterText = "";
}

GravityDataProduct::~GravityDataProduct()
{
	delete gravityDataProductPB;
}

string GravityDataProduct::getDataProductID()
{
	return gravityDataProductPB->dataproductid();
}

void GravityDataProduct::setFilterText(string filterText)
{
	this->filterText = filterText;
}

string GravityDataProduct::getFilterText()
{
	return filterText;
}

void GravityDataProduct::setSoftwareVersion(string softwareVersion)
{
	gravityDataProductPB->set_softwareversion(softwareVersion);
}

string GravityDataProduct::getSoftwareVersion()
{
	return gravityDataProductPB->softwareversion();
}

uint64_t GravityDataProduct::getGravityTimestamp()
{
	return gravityDataProductPB->timestamp();
}

void GravityDataProduct::setData(void* data, int size)
{
	gravityDataProductPB->add_data(data, size);
}

void GravityDataProduct::setData(const google::protobuf::Message& data)
{
	char* vdata = (char*)malloc(data.ByteSize());
	data.SerializeToArray(vdata, data.ByteSize());
	gravityDataProductPB->add_data(vdata, data.ByteSize());
	delete vdata;
}

bool GravityDataProduct::getData(void* data, int size)
{
	memcpy(data, gravityDataProductPB->mutable_data()->mutable_data(), size);

	return true;
}

int GravityDataProduct::getDataSize()
{
	return gravityDataProductPB->data_size();
}

bool GravityDataProduct::populateMessage(google::protobuf::Message& data)
{
	data.ParseFromArray(gravityDataProductPB->mutable_data()->mutable_data(), getDataSize());

	return true;
}

} /* namespace gravity */
