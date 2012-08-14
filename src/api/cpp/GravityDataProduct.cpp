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

uint64_t GravityDataProduct::getTimestamp()
{
	return gravityDataProductPB->timestamp();
}

void GravityDataProduct::setData(void* data, int len)
{
	gravityDataProductPB->add_data(data, len);
}

void GravityDataProduct::setData(google::protobuf::Message& data)
{
	void vdata[data.ByteSize()];
	data.SerializeToArray(vdata, data.ByteSize());
	gravityDataProductPB->add_data(vdata, data.ByteSize());
	delete vdata;
}

void* GravityDataProduct::getData()
{
	return gravityDataProductPB->mutable_data()->mutable_data();
}

int GravityDataProduct::getDataSize()
{
	return gravityDataProductPB->data_size();
}

void GravityDataProduct::populateMessage(google::protobuf::Message& data)
{
	data.ParseFromArray(getData(), getDataSize());
}

} /* namespace gravity */
