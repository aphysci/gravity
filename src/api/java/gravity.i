
%include "enums.swg"
%include "std_string.i" // for std::string typemaps

%javaconst(1);
%{
#include "GravityNode.h"
#include "CPPGravitySubscriber.h"
%}

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("gravity_wrap");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
%}

//%rename(CPPGravitySubscriber) CPPGravitySubscriber; 

// (2.2)
%typemap(jstype) const gravity::GravitySubscriber& "GravitySubscriber";

// (2.3)
%typemap(javainterfaces) GravitySubscriber "GravitySubscriber"

// (2.5)
//%typemap(javaout,pgcppname="n",
//         pre="    CPPGravitySubscriber n = gravity.makeNative($javainput);")
//        GravitySubscriber  "CPPGravitySubscriber.getCPtr(n)"

%typemap(javain,pgcppname="n",
         pre="    CPPGravitySubscriber n = gravity.makeNative($javainput);")
        const gravity::GravitySubscriber&  "CPPGravitySubscriber.getCPtr(n)"

%pragma(java) moduleimports="import gravity.GravityDataProduct;" 
%pragma(java) modulecode=%{

  // (2.4)
  private static class CPPGravitySubscriberProxy extends CPPGravitySubscriber {
    private GravitySubscriber delegate;
    public CPPGravitySubscriberProxy(GravitySubscriber i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public void subscriptionFilled(final GravityDataProduct dataProduct) {
      delegate.subscriptionFilled(dataProduct);
    }
  }

  // (2.5)
  public static CPPGravitySubscriber makeNative(GravitySubscriber i) {
    if (i instanceof CPPGravitySubscriber) {
      // If it already *is* a CPPGravitySubscriber don't bother wrapping it again
      return (CPPGravitySubscriber)i;
    }
    return new CPPGravitySubscriberProxy(i);
  }
%}

%module gravity

namespace gravity {

//	class GravitySubscriber {
//		virtual void subscriptionFilled(const GravityDataProduct& dataProduct) = 0;
//	};

	class CPPGravitySubscriber {
		void subscriptionFilled(const GravityDataProduct& dataProduct);
	};

    enum GravityReturnCode {
        SUCCESS = 0,
        FAILURE = -1,
        NO_SERVICE_DIRECTORY = -2,
        REQUEST_TIMEOUT = -3,
        DUPLICATE = -4,
        REGISTRATION_CONFLICT = -5,
        NOT_REGISTERED = -6,
        NO_SUCH_SERVICE = -7,
        NO_SUCH_DATA_PRODUCT = -8,
        LINK_ERROR = -9,
        INTERRUPTED = -10
    };

class GravityNode {
public:    
	GravityNode();
	~GravityNode();
    GravityReturnCode init();
    GravityReturnCode registerDataProduct(const std::string& dataProductID, unsigned short networkPort, const std::string &transportType);
    GravityReturnCode unregisterDataProduct(const std::string& dataProductID);
    GravityReturnCode subscribe(const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter);

};
};
