
%include "enums.swg"
%include "std_string.i" // for std::string typemaps
%include "various.i"

%module(directors="1") gravity

%javaconst(1);
%{
#include "GravityNode.h"
#include "CPPGravitySubscriber.h"
#include "CPPGravityServiceProvider.h"
#include "CPPGravityRequestor.h"
#include "CPPGravityLogger.h"
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
* GravitySubscriber conversion
*******/
%typemap(jstype) const gravity::GravitySubscriber& "GravitySubscriber";
%typemap(javainterfaces) GravitySubscriber "GravitySubscriber"

%typemap(javain,pgcppname="n",
         pre="    CPPGravitySubscriber n = gravity.makeNativeSubscriber($javainput);")
        const gravity::GravitySubscriber&  "CPPGravitySubscriber.getCPtr(n)"

/******
* GravityRequestor conversion
*******/
%typemap(jstype) const gravity::GravityRequestor& "GravityRequestor";
%typemap(javainterfaces) GravityRequestor "GravityRequestor"

%typemap(javain,pgcppname="n",
         pre="    CPPGravityRequestor n = gravity.makeNativeRequestor($javainput);")
        const gravity::GravityRequestor&  "CPPGravityRequestor.getCPtr(n)"

/******
* GravityServiceProvider conversion
*******/
%typemap(jstype) const gravity::GravityServiceProvider& "GravityServiceProvider";
%typemap(javainterfaces) GravityServiceProvider "GravityServiceProvider"

%typemap(javain,pgcppname="n",
         pre="    CPPGravityServiceProvider n = gravity.makeNativeProvider($javainput);")
        const gravity::GravityServiceProvider&  "CPPGravityServiceProvider.getCPtr(n)"

/******
* Logger conversion
*******/
%typemap(jstype) gravity::Logger* "Logger";
%typemap(javainterfaces) Logger "Logger"

%typemap(javain,pgcppname="n",
         pre="    CPPGravityLogger n = gravity.makeNativeLogger($javainput);")
        gravity::Logger*  "CPPGravityLogger.getCPtr(n)"


/******
* All the required typemaps to allow a GDP to be passed from Java to C++ (being serialized to a byte array in between)
*******/
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
import com.aphysci.gravity.GravityRequestor;
import com.aphysci.gravity.GravityServiceProvider;
import com.aphysci.gravity.Logger;
%}
//%typemap(javaimports) gravity::CPPGravitySubscriber %{
//import com.aphysci.gravity.GravityDataProduct;
//import com.aphysci.gravity.GravitySubscriber;
//import com.aphysci.gravity.GravityRequestor;
//import com.aphysci.gravity.GravityServiceProvider;
//import com.aphysci.gravity.Logger;
//%}
%typemap(javaimports) gravity::Log %{
import com.aphysci.gravity.Logger;
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

/*****
* typemaps to convert handle return of GDP from java method
******/
%typemap(directorout) shared_ptr<gravity::GravityDataProduct> {
    signed char* data = JCALL2(GetByteArrayElements, jenv, $input, NULL);
    int length = JCALL1(GetArrayLength, jenv, $input);
	shared_ptr<gravity::GravityDataProduct> ret(new gravity::GravityDataProduct((void *)data, length));
	JCALL3(ReleaseByteArrayElements, jenv, $input, data, JNI_ABORT); 
    $result = ret;
    (jenv)->DeleteLocalRef(jBYTE);
}
%typemap(javadirectorout) shared_ptr<gravity::GravityDataProduct> "$javacall"
%typemap(jni) shared_ptr<gravity::GravityDataProduct> "jbyteArray"
%typemap(jstype) shared_ptr<gravity::GravityDataProduct> "byte[]"
%typemap(jtype) shared_ptr<gravity::GravityDataProduct> "byte[]"
%typemap(javaout) shared_ptr<gravity::GravityDataProduct> {
    return $jnicall;
  }
%typemap(javain) shared_ptr<gravity::GravityDataProduct> "$javainput"
%typemap(javadirectorin) shared_ptr<gravity::GravityDataProduct> "$jniinput"
%typemap(directorin, descriptor="[B") shared_ptr<gravity::GravityDataProduct> {} 

%include "logger.i"
%include "modulecode.i"

// this turns on director features for CPPGravitySubscriber
%feature("director") gravity::CPPGravitySubscriber;
%feature("director") gravity::CPPGravityRequestor;
%feature("director") gravity::CPPGravityServiceProvider;

// This is where we actually declare the types and methods that will be made available in Java.  This section must be kept in
// sync with the Gravity API.
namespace gravity {

	class CPPGravitySubscriber {
	public:
		int subscriptionFilled(char *BYTE, int length);
	};

	class CPPGravityRequestor {
	public:
		int requestFilled(const std::string& serviceID, const std::string& requestID, char *BYTE, int length);
	};

	class CPPGravityServiceProvider {
	public:
		shared_ptr<gravity::GravityDataProduct> request(char *BYTE, int length);
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
    GravityReturnCode subscribe(const std::string& connectionURL, const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, 
            const std::string& filter = "");
    GravityReturnCode unsubscribe(const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter = "");
	GravityReturnCode publish(const gravity::GravityDataProduct& dataProduct, const std::string& filter = "");
	GravityReturnCode request(const std::string& serviceID, const gravity::GravityDataProduct& dataProduct, 
	        const gravity::GravityRequestor& requestor, const std::string& requestID = "");
	GravityReturnCode request(const std::string& connectionURL, const std::string& serviceID, const const gravity::GravityDataProduct& dataProduct,
            const const gravity::GravityRequestor& requestor, const std::string& requestID = emptyString);
    GravityReturnCode registerService(const std::string& serviceID, short networkPort,
            const std::string& transportType, const gravity::GravityServiceProvider& server);
    GravityReturnCode unregisterService(const std::string& serviceID);
            
};

};
