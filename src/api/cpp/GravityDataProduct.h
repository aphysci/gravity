/*
 * GravityDataProduct.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYDATAPRODUCT_H_
#define GRAVITYDATAPRODUCT_H_

namespace gravity
{

using namespace std;

class GravityDataProduct
{
private:
	GravityDataProductPB* gravityDataProductPB;
	string filterText;
public:
	GravityDataProduct(string dataProductID);
	virtual ~GravityDataProduct();

	uint64_t getTimestamp();
	string getDataProductID();
	void setSoftwareVersion(string softwareVersion);
	string getSoftwareVersion();
	void setFilterText(string filterText);
	string getFilterText();

	void setData(void* data, int len);
	void setData(google::protobuf::Message& data);
	void* getData();
	int getDataSize();
	void populateMessage(google::protobuf::Message& data);
};

} /* namespace gravity */
#endif /* GRAVITYDATAPRODUCT_H_ */
