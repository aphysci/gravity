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

// GravityInteractor.h

#pragma once

using namespace System;
#include "Stdafx.h"

#include <GravityNode.h>

namespace GravityCS
{
using namespace std;

//Forward Declarations.
ref class DataProduct;
public delegate void RequestFilled(String^ serviceID, String^ requestID, const GravityCS::DataProduct^ response);
public delegate void SubscriptionFilled(Collections::Generic::IList<DataProduct^>^ dataProducts);
public delegate DataProduct^ ServiceRequest(String^ serviceID, const DataProduct^ dataProducts);

std::string Marshall(String^ managedString);

public enum class GravityTransportType
{
      TCP = 0,
      INPROC = 1,
      PGM = 2,
      EPGM= 3,
#ifndef WIN32
      IPC = 4
#endif
};

public ref class GravityInteractor
{
private:
	gravity::GravityNode* gn;
public:

	void Init(String^ componentName);
	bool Subscribe(String^ name, SubscriptionFilled^ callback, String^ filter);
	bool Subscribe(String^ name, SubscriptionFilled^ callback)
	{
		return Subscribe(name, callback, "");
	}
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

	//Register/Unregister
	void RegisterDataProduct(String^ dataProductID, GravityTransportType transportType);
	void Publish(const DataProduct^ dataProduct, String^ filter);
	void Publish(const DataProduct^ dataProduct)
	{
		Publish(dataProduct, "");
	}
	void UnregisterDataProduct(String^ dataProductID);

    void RegisterService(String^ serviceID, GravityTransportType transportType, ServiceRequest^ serverFunction);
	void UnregisterService(String^ serviceID);
};


public ref class DataProduct
{
internal:
	DataProduct(shared_ptr<gravity::GravityDataProduct> cpp_dataProduct)
	{
		this->cpp_dataProduct = new shared_ptr<gravity::GravityDataProduct>(cpp_dataProduct);
	}

	//Protobuf object
	shared_ptr<gravity::GravityDataProduct>* cpp_dataProduct; //This is convoluted but we can't have non-pointer, non-clr C++ types in a .net class.
public:
	//DataProduct();
	DataProduct(String^ dataProductID)
	{
		this->cpp_dataProduct = new shared_ptr<gravity::GravityDataProduct>(new gravity::GravityDataProduct(Marshall(dataProductID)));
	}

	void setData(Google::ProtocolBuffers::IMessage^ data)
	{
		array<unsigned char>^ bytesArray = data->ToByteArray();
		pin_ptr<unsigned char> dataPointer = &bytesArray[0]; //Pin the array while we copy it.

		(*cpp_dataProduct)->setData(dataPointer, bytesArray->Length);
	}

	void getProtobufObject(Google::ProtocolBuffers::IBuilder^ protobufOut)
	{
		array<unsigned char>^ bytesArray = gcnew array<unsigned char>((*cpp_dataProduct)->getDataSize()); //No need to free this guy, right.

		{
			pin_ptr<unsigned char> dataPointer = &bytesArray[0]; //Pin the array while we copy it.
			(*cpp_dataProduct)->getData(dataPointer, (*cpp_dataProduct)->getDataSize());
		}

		Google::ProtocolBuffers::ByteString^ bs = Google::ProtocolBuffers::ByteString::CopyFrom(bytesArray);
		protobufOut->WeakMergeFrom(bs);
	}

	array<unsigned char>^ getRawData()
	{
		array<unsigned char>^ bytesArray = gcnew array<unsigned char>((*cpp_dataProduct)->getDataSize()); //No need to free this guy, right.

		{
			pin_ptr<unsigned char> dataPointer = &bytesArray[0]; //Pin the array while we copy it.
			(*cpp_dataProduct)->getData(dataPointer, (*cpp_dataProduct)->getDataSize());
		}

		return bytesArray;
	}

	~DataProduct()
	{
		delete cpp_dataProduct;
	}

	//TODO:
	// getTimestamp();
	// getDataProductID();
	// setDataProductID();
	// etc.
};

}
