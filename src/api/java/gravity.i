
%include "enums.swg"
%include "std_string.i" // for std::string typemaps
%include "various.i"

%module(directors="1") gravity

%javaconst(1);
%{
#include "GravityNode.h"
#include "CPPGravitySubscriber.h"
%}

// load the shared lib in the generated code
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

/******
* All the required typemaps to allow a GDP to be passed from Java to C++ (being serialized to a byte array in between)
*******/
%typemap(jstype) const gravity::GravitySubscriber& "GravitySubscriber";
%typemap(javainterfaces) GravitySubscriber "GravitySubscriber"

%typemap(javain,pgcppname="n",
         pre="    CPPGravitySubscriber n = gravity.makeNative($javainput);")
        const gravity::GravitySubscriber&  "CPPGravitySubscriber.getCPtr(n)"

%typemap(jtype) const gravity::GravityDataProduct& "byte[]";
%typemap(jstype) const gravity::GravityDataProduct& "GravityDataProduct";
%typemap(jni) const gravity::GravityDataProduct&  "jbyteArray"
%typemap(javain) const gravity::GravityDataProduct&  "$javainput.serializeToArray()"

%typemap(in) const gravity::GravityDataProduct&  {
    signed char* data = JCALL2(GetByteArrayElements, jenv, $input, NULL);
    int length = JCALL1(GetArrayLength, jenv, $input);
	$1 = new gravity::GravityDataProduct((void *)data, length);
	JCALL3(ReleaseByteArrayElements, jenv, $input, data, JNI_ABORT); 
}

// this frees the memory allocated in the 'in' typemap above.
%typemap(freearg) const gravity::GravityDataProduct&  {
	delete $1;
}
/******
* End GDP typemaps
*******/


%typemap(javaimports) gravity::GravityNode %{
import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
%}
%typemap(javaimports) gravity::CPPGravitySubscriber %{
import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
%}

%typemap(directorin, descriptor="[B") char *BYTE {
    // length var is assumed to be passed into the function as well
    jbyteArray jb = (jenv)->NewByteArray(length);
    (jenv)->SetByteArrayRegion(jb, 0, length, (jbyte*)BYTE);
    $input = jb;
    // can't deallocate here because it hasn't been used yet.
//    (jenv)->DeleteLocalRef(jb);
}

// egregious hack alert - the below typemap is added just to deallocate the memory for
// the java byte array allocated above.  Unfortunately there doesn't seem to be a freearg
// equivalent for directorin typemaps.
%typemap(directorout) int %{
    $result = ($1_ltype)$input;
    (jenv)->DeleteLocalRef(jBYTE);
%}

%typemap(javadirectorin) char *BYTE "$jniinput"
%typemap(javadirectorout) char *BYTE "$javacall"

// imports for gravity.java
%pragma(java) moduleimports=%{
import java.util.WeakHashMap;
import java.util.Map;
import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
%}
 
// code for gravity.java that creates a proxy class for emulating a Java interface to a C++ class.
%pragma(java) modulecode=%{
  private static Map<GravitySubscriber, CPPGravitySubscriberProxy> proxyMap = 
            new WeakHashMap<GravitySubscriber, CPPGravitySubscriberProxy>();
  
  private static class CPPGravitySubscriberProxy extends CPPGravitySubscriber {
    private GravitySubscriber delegate;
    public CPPGravitySubscriberProxy(GravitySubscriber i) {
      delegate = i;
    }

    @SuppressWarnings("unused")
    public int subscriptionFilled(byte[] arr, int length) {
      System.out.println("made it to CPPGravitySubscriberProxy.subscriptionFilled");
      delegate.subscriptionFilled(new GravityDataProduct(arr));
      return 0;
    }
  }

  public static CPPGravitySubscriber makeNative(GravitySubscriber i) {
    if (i instanceof CPPGravitySubscriber) {
      // If it already *is* a CPPGravitySubscriber don't bother wrapping it again
      return (CPPGravitySubscriber)i;
    }
    CPPGravitySubscriberProxy proxy = proxyMap.get(i);
    if (proxy == null) {
      proxy = new CPPGravitySubscriberProxy(i);
      proxyMap.put(i, proxy);
    }
    return proxy;
  }
%}

// this turns on director features for CPPGravitySubscriber
%feature("director") gravity::CPPGravitySubscriber;


// This is where we actually declare the types and methods that will be made available in Java.  This section must be kept in
// sync with the Gravity API.
namespace gravity {

	class CPPGravitySubscriber {
	public:
		virtual ~CPPGravitySubscriber();
		virtual int subscriptionFilled(char *BYTE, int length);
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
        INTERRUPTED = -10,
        NO_SERVICE_PROVIDER = -11
    };

class GravityNode {
public:    
	GravityNode();
	~GravityNode();
    GravityReturnCode init();
    GravityReturnCode registerDataProduct(const std::string& dataProductID, unsigned short networkPort, const std::string &transportType);
    GravityReturnCode unregisterDataProduct(const std::string& dataProductID);
    GravityReturnCode subscribe(const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter = "");
    GravityReturnCode subscribe(const std::string& connectionURL, const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter = "");
    GravityReturnCode unsubscribe(const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter=emptyString);
	GravityReturnCode publish(const gravity::GravityDataProduct& dataProduct, const std::string& filter = "");
};
};
