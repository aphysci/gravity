
%include "enums.swg"
%include "std_string.i" // for std::string typemaps
%include "various.i"

%module(directors="1") gravity

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

%typemap(jstype) const gravity::GravitySubscriber& "GravitySubscriber";
%typemap(javainterfaces) GravitySubscriber "GravitySubscriber"

%typemap(javain,pgcppname="n",
         pre="    CPPGravitySubscriber n = gravity.makeNative($javainput);")
        const gravity::GravitySubscriber&  "CPPGravitySubscriber.getCPtr(n)"

%typemap(jtype) const gravity::GravityDataProduct& "byte[]";
%typemap(jstype) const gravity::GravityDataProduct& "GravityDataProductPB";
%typemap(jni) const gravity::GravityDataProduct&  "jbyteArray"
%typemap(javain) const gravity::GravityDataProduct&  "$javainput.toByteArray()"

%typemap(in) const gravity::GravityDataProduct&  {
    signed char* data = JCALL2(GetByteArrayElements, jenv, $input, NULL);
    int length = JCALL1(GetArrayLength, jenv, $input);
	$1 = new gravity::GravityDataProduct((void *)data, length);
	JCALL3(ReleaseByteArrayElements, jenv, $input, data, JNI_ABORT); 
}

%typemap(freearg) const gravity::GravityDataProduct&  {
	delete $1;
}

%typemap(javaimports) gravity::GravityNode %{
import com.aphysci.gravity.protobuf.GravityDataProduct.GravityDataProductPB;
import com.aphysci.gravity.GravitySubscriber;
%}
%typemap(javaimports) gravity::CPPGravitySubscriber %{
import com.aphysci.gravity.protobuf.GravityDataProduct.GravityDataProductPB;
import com.aphysci.gravity.GravitySubscriber;
%}

//%typemap(directorin, descriptor="[B") (const signed char* arr, int length) {
//    jbyteArray jb = (jenv)->NewByteArray(length);
//    (jenv)->SetByteArrayRegion(jb, 0, length, (jbyte*)arr);
//    $input = jb;
//}

//%typemap(directorout) (const signed char* arr, int length) {
//	$1 = 0;
//	if($input){
//		$result = (char *) jenv->GetByteArrayElements($input, 0);
//		if(!$1)
//			return $null;
//	}
//}

//%typemap(javadirectorin) (const signed char* arr, int length) "$jniinput"
//%typemap(javadirectorout) (const signed char* arr, int length) "$javacall"

//"jbyteArray";

%pragma(java) moduleimports=%{
import com.aphysci.gravity.protobuf.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
%}
 
%pragma(java) modulecode=%{

  private static class CPPGravitySubscriberProxy extends CPPGravitySubscriber {
    private GravitySubscriber delegate;
    public CPPGravitySubscriberProxy(GravitySubscriber i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public void subscriptionFilled(SWIGTYPE_p_signed_char arr, int length) {
      System.out.println("made it to CPPGravitySubscriberProxy.subscriptionFilled");
      delegate.subscriptionFilled(null);
    }
  }

  public static CPPGravitySubscriber makeNative(GravitySubscriber i) {
    if (i instanceof CPPGravitySubscriber) {
      // If it already *is* a CPPGravitySubscriber don't bother wrapping it again
      return (CPPGravitySubscriber)i;
    }
    return new CPPGravitySubscriberProxy(i);
  }
%}

%feature("director") gravity::CPPGravitySubscriber;

namespace gravity {

	class CPPGravitySubscriber {
	public:
		virtual ~CPPGravitySubscriber();
		virtual void subscriptionFilled(const signed char* arr, int length);
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
	GravityReturnCode publish(const gravity::GravityDataProduct& dataProduct);
};
};
