// GravityInteractor.h

#pragma once

using namespace System;
#include "Stdafx.h"

#include <GravityNode.h>

namespace GravityCS
{

//Forward Declarations.  
ref class DataProduct;
public delegate void RequestFilled(String^ serviceID, String^ requestID, const GravityCS::DataProduct^ response);
public delegate void SubscriptionFilled(Collections::Generic::IList<DataProduct^>^ dataProducts);

std::string Marshall(String^ managedString);

public ref class GravityInteractor
{
private:
	gravity::GravityNode* gn;
public:

	void Init(String^ componentName);
	bool Subscribe(String^ name, SubscriptionFilled^ callback, String^ filter);
	void Unsubscribe(String^ name, SubscriptionFilled^ callback);

	void Request(String^ service_ID, const DataProduct^ request, RequestFilled^ callbacks, String^ request_ID, int timeout_in_milliseconds);
	//Simple Wrappers for default arguments.  
	void Request(String^ service_ID, const DataProduct^ request, RequestFilled^ callbacks)
	{
		Request(service_ID, request, callbacks, "", -1);
	}
	void Request(String^ service_ID, const DataProduct^ request, RequestFilled^ callbacks, String^ request_ID)
	{
		Request(service_ID, request, callbacks, request_ID, -1);
	}

	IntPtr GetGravityPtr()
	{
		return (IntPtr) gn;
	}
};


public ref class DataProduct
{
internal:
	DataProduct(gravity::GravityDataProduct* cpp_dataProduct)
	{
		this->cpp_dataProduct = cpp_dataProduct;
		we_own_the_dataproduct = false;
	}

	//Protobuf object
	gravity::GravityDataProduct* cpp_dataProduct;
	bool we_own_the_dataproduct;
public:
	//DataProduct();
	DataProduct(String^ dataProductID)
	{
		cpp_dataProduct = new gravity::GravityDataProduct(Marshall(dataProductID));
		we_own_the_dataproduct = true;
	}

	void setData(Google::ProtocolBuffers::IMessage^ data)
	{
		array<unsigned char>^ bytesArray = data->ToByteArray();
		pin_ptr<unsigned char> dataPointer = &bytesArray[0]; //Pin the array while we copy it.  

		this->cpp_dataProduct->setData(dataPointer, bytesArray->Length);
	}

	void getProtobufObject(Google::ProtocolBuffers::IBuilder^ protobufOut)
	{
		array<unsigned char>^ bytesArray = gcnew array<unsigned char>(cpp_dataProduct->getDataSize()); //No need to free this guy, right.  

		{
			pin_ptr<unsigned char> dataPointer = &bytesArray[0]; //Pin the array while we copy it.  
			cpp_dataProduct->getData(dataPointer, cpp_dataProduct->getDataSize());
		}

		Google::ProtocolBuffers::ByteString^ bs = Google::ProtocolBuffers::ByteString::CopyFrom(bytesArray);
		protobufOut->WeakMergeFrom(bs);
	}

	array<unsigned char>^ getRawData()
	{
		array<unsigned char>^ bytesArray = gcnew array<unsigned char>(cpp_dataProduct->getDataSize()); //No need to free this guy, right.  

		{
			pin_ptr<unsigned char> dataPointer = &bytesArray[0]; //Pin the array while we copy it.  
			cpp_dataProduct->getData(dataPointer, cpp_dataProduct->getDataSize());
		}

		return bytesArray;
	}

	~DataProduct()
	{
		if(we_own_the_dataproduct)
			delete cpp_dataProduct;
	}

	//TODO: 
	// getTimestamp();
	// getDataProductID();
	// setDataProductID();
	// etc.  
};

}