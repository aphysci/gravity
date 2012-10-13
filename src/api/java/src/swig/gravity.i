
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
#include "CPPGravityHeartbeatListener.h"
#include "CPPGravityLogger.h"
%}

// load the shared lib in the generated code
%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("zmq");
    } catch (UnsatisfiedLinkError unused) {
	    try {
	        System.loadLibrary("libzmq");
	    } catch (UnsatisfiedLinkError e) {
	      System.err.println("Native code library failed to load. Tried both zmq and libzmq.\n" + e);
	      System.exit(1);
	    }
    }

    try {
        System.loadLibrary("gravity_wrap");
    } catch (UnsatisfiedLinkError unused) {
	    try {
	        System.loadLibrary("libgravity_wrap");
	    } catch (UnsatisfiedLinkError e) {
	      System.err.println("Native code library failed to load. Tried both gravity_wrap and libgravity_wrap.\n" + e);
	      System.exit(1);
	    }
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
 * GravityHeartbeatListener conversion
 *******/
%typemap(jstype) const gravity::GravityHeartbeatListener& "GravityHeartbeatListener";
%typemap(javainterfaces) GravityHeartbeatListener "GravityHeartbeatListener"

%typemap(javain,pgcppname="n",
         pre="    CPPGravityHeartbeatListener n = gravity.makeNativeHeartbeatListener($javainput);")
        const gravity::GravityHeartbeatListener&  "CPPGravityHeartbeatListener.getCPtr(n)"

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

/*****
 * Typemaps to handle passing a byte array from C++ into Java.  Conversion between
 * GDP and byte array is handled outside of this interface code (unlike the conversion
 * when passing GDP's from Java to C++).
 *****/
%typemap(directorin, descriptor="[B") char *BYTE {
    // length var is assumed to be passed into the function as well
    jbyteArray jb = (jenv)->NewByteArray(length);
    (jenv)->SetByteArrayRegion(jb, 0, length, (jbyte*)BYTE);
    $input = jb;
    // can't deallocate here because it hasn't been used yet.
//    (jenv)->DeleteLocalRef(jb);
}

// The below typemap is added just to deallocate the memory for
// the java byte array allocated above.  Unfortunately there doesn't seem to be a freearg
// equivalent for directorin typemaps.
%typemap(directorout) int %{
    $result = ($1_ltype)$input;
    (jenv)->DeleteLocalRef(jBYTE);
%}
%typemap(javadirectorin) char *BYTE "$jniinput"
%typemap(javadirectorout) char *BYTE "$javacall"

/*****
 * typemaps to convert handle return of GDP from java method so that it can travel across jni boundary as a byte[]
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
%typemap(jtype) shared_ptr<gravity::GravityDataProduct> "byte[]"
%typemap(jstype) shared_ptr<gravity::GravityDataProduct> "byte[]"
%typemap(javaout) shared_ptr<gravity::GravityDataProduct> {
    return $jnicall;
  }

%typemap(javain) shared_ptr<gravity::GravityDataProduct> "$javainput"
%typemap(javadirectorin) shared_ptr<gravity::GravityDataProduct> "$jniinput"
%typemap(directorin, descriptor="[B") shared_ptr<gravity::GravityDataProduct> {} 

/******
 * When gravity::GravityNode::request returns shared_ptr<GravityDataProduct> though, we want to
 * convert that to a GravityDataProduct on the java side of the JNI interface so that users
 * get a GDP rather than a byte[]. 
 ******/
%typemap(jstype) shared_ptr<gravity::GravityDataProduct> gravity::GravityNode::request "GravityDataProduct"
%typemap(javaout) shared_ptr<gravity::GravityDataProduct> gravity::GravityNode::request {
    return new GravityDataProduct($jnicall);
  }

%include "modulecode.i"
%include "logger.i"
%include "gravitynode.i"
