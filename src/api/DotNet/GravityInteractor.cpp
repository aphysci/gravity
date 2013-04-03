// This is the main DLL file.

#include "stdafx.h"

#include <msclr\marshal_cppstd.h> //For String Marshalling

#include <list>

#include "GravityInteractor.h"
#include <GravityRequestor.h>
#include <GravitySubscriber.h>

namespace GravityCS {

using namespace std;

std::string Marshall(String^ managedString)
{
	msclr::interop::marshal_context context;
    return context.marshal_as<std::string>(managedString);
}

void GravityInteractor::Init(String^ componentName)
{
	std::string name_str = Marshall(componentName);
	gn = new gravity::GravityNode();
	gn->init(name_str);
}

class genericSubscribeWrapper;

ref class subscriberWrapperHelper 
{
internal:
	static Collections::Generic::Dictionary<IntPtr, SubscriptionFilled^>^ hashtable = gcnew Collections::Generic::Dictionary<IntPtr, SubscriptionFilled^>();
	static SubscriptionFilled^ GetDelegateForSubscriber(genericSubscribeWrapper* subscriber)
	{
		SubscriptionFilled^ ret_val;
		if(!hashtable->TryGetValue((IntPtr)subscriber, ret_val))
			throw gcnew System::ArgumentException("Could not find subscriber delegate");
		return ret_val;
	}
};

class genericSubscribeWrapper : public gravity::GravitySubscriber
{
public:
	genericSubscribeWrapper(GravityCS::SubscriptionFilled^ callbacks)
	{
		subscriberWrapperHelper::hashtable->Add((IntPtr)this, callbacks);
	}


	virtual void subscriptionFilled(const std::vector< std::shared_ptr<gravity::GravityDataProduct> >& dataProducts)
	{
		System::Collections::Generic::List<DataProduct^>^ dataProducts_CS = gcnew System::Collections::Generic::List<DataProduct^>(dataProducts.size());

		for(size_t i = 0; i < dataProducts.size(); i++)
		{
			//Convert Data Product
			GravityCS::DataProduct^ dataProduct = gcnew GravityCS::DataProduct(const_cast<gravity::GravityDataProduct*>(&(*dataProducts[i])));
			dataProducts_CS->Add(dataProduct);
		}

		//Call the delegate.  
		subscriberWrapperHelper::GetDelegateForSubscriber(this)->Invoke(dataProducts_CS); //TODO: make sure this blocks until finished.  
	}

	~genericSubscribeWrapper()
	{
		//Clean ourselves up.  
		subscriberWrapperHelper::hashtable->Remove((IntPtr)this);
	}
};

bool GravityInteractor::Subscribe(String^ name, SubscriptionFilled^ callback, String^ filter)
{
	//Convert strings.  
	std::string name_str = Marshall(name);
	std::string filter_str = Marshall(filter);

	genericSubscribeWrapper* gs = new genericSubscribeWrapper(callback);
	return gn->subscribe(name_str, *gs, filter_str) == gravity::GravityReturnCode::SUCCESS;
}

void GravityInteractor::Unsubscribe(String^ name, SubscriptionFilled^ callback)
{
	std::string name_str = Marshall(name);
	
	gravity::GravitySubscriber* sub;
	Collections::Generic::Dictionary<IntPtr, SubscriptionFilled^>::Enumerator^ e = subscriberWrapperHelper::hashtable->GetEnumerator();
	
	while(e->MoveNext() != false)
	{
		if(e->Current.Value == callback)
		{
			sub = (gravity::GravitySubscriber*) ((IntPtr) e->Current.Key).ToPointer();
			subscriberWrapperHelper::hashtable->Remove((IntPtr)e->Current.Key);
		}
	}

	gn->unsubscribe(name_str, *sub);
	delete sub;
}

//class: genericRequestWrapperCS 
// A C++ wrapper that calls the C# callbacks.  
//
class genericRequestWrapperCS; //Forward Declararion

ref class requestWrapperHelper
{
internal:
	static Collections::Generic::Dictionary<IntPtr, RequestFilled^>^ hashtable = gcnew Collections::Generic::Dictionary<IntPtr, RequestFilled^>();
	static RequestFilled^ GetDelegateFromRequestor(genericRequestWrapperCS* requestor)
	{
		RequestFilled^ ret_val;
		if(!hashtable->TryGetValue((IntPtr)requestor, ret_val))
			throw gcnew System::ArgumentException("Could not find requestor delegate");
		return ret_val;
	}
};


class genericRequestWrapperCS : public gravity::GravityRequestor
{
public:
	genericRequestWrapperCS(GravityCS::RequestFilled^ callbacks)
	{
		requestWrapperHelper::hashtable->Add((IntPtr)this, callbacks);
	}

    virtual void requestFilled(string service_ID_str, string request_ID_str, const gravity::GravityDataProduct& response)
	{
		//Marshall Strings
		String^ serviceID = gcnew String(service_ID_str.c_str());
		String^ requestID = gcnew String(request_ID_str.c_str());
		//Convert Data Product
		const GravityCS::DataProduct^ dataProduct = gcnew GravityCS::DataProduct(const_cast<gravity::GravityDataProduct*>(&response));

		//Call the delegate.  
		requestWrapperHelper::GetDelegateFromRequestor(this)->Invoke(serviceID, requestID, dataProduct); //TODO: make sure this blocks until finished.  

		//Clean ourselves up.  
		requestWrapperHelper::hashtable->Remove((IntPtr)this);
		delete this;
	}
};

// A C# wrapper for C++ GravityRequest.  
void GravityInteractor::Request(String^ service_ID, const DataProduct^ request, RequestFilled^ callbacks, String^ request_ID, int timeout_in_milliseconds)
{
	//Convert strings.  
	std::string service_ID_str = Marshall(service_ID);
	std::string request_ID_str = Marshall(request_ID);

	//Create Request Wrapper.  
	genericRequestWrapperCS* callbackWrapper = new genericRequestWrapperCS(callbacks);

	//Call Gravity
	gn->request(service_ID_str, *(request->cpp_dataProduct), *callbackWrapper, request_ID_str, timeout_in_milliseconds);
}

} //namespace
