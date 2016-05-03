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


%include "enums.swg"
%include "std_string.i" // for std::string typemaps
%include "various.i"
//%include "stdint.i"
//%include "typemaps.i"

%module(directors="1") gravity

%javaconst(1);
%{
#include "GravityNode.h"
#include "FutureResponse.h"
#include "CPPGravitySubscriber.h"
#include "CPPGravityServiceProvider.h"
#include "CPPGravityRequestor.h"
#include "CPPGravityHeartbeatListener.h"
#include "CPPGravityLogger.h"
using namespace std::tr1;
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
 * map int64 to java long
 ******

%define INOUT_TYPEMAP(TYPE, JNITYPE, JTYPE, JAVATYPE, JNIDESC, TYPECHECKTYPE)
INOUT_TYPEMAP(int64_t, jlong, long, Long, "[Ljava/lang/Long;", jlongArray);
*/
%typemap(jni) int64_t *INOUT, int64_t &INOUT %{jlongArray%}
%typemap(jtype) int64_t *INOUT, int64_t &INOUT "long[]"
%typemap(jstype) int64_t *INOUT, int64_t &INOUT "long[]"
%typemap(javain) int64_t *INOUT, int64_t &INOUT "$javainput"
%typemap(javadirectorin) int64_t *INOUT, int64_t &INOUT "$jniinput"
%typemap(javadirectorout) int64_t *INOUT, int64_t &INOUT "$javacall"

%typemap(in) int64_t *INOUT, int64_t &INOUT {
  if (!$input) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
    return $null;
  }
  if (JCALL1(GetArrayLength, jenv, $input) == 0) {
    SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
    return $null;
  }
  $1 = ($1_ltype) JCALL2(GetLongArrayElements, jenv, $input, 0);
}

%typemap(freearg) int64_t *INOUT, int64_t &INOUT ""

%typemap(argout) int64_t *INOUT, int64_t &INOUT
{ JCALL3(ReleaseLongArrayElements, jenv, $input, (jlong *)$1, 0); }

//%typemap(directorout,warning="Need to provide int64_t *INOUT directorout typemap") int64_t *INOUT {
//}

%typemap(directorin,descriptor="[J") int64_t &INOUT
%{
    jlongArray jb = (jenv)->NewLongArray(1);
    (jenv)->SetLongArrayRegion(jb, 0, 1, (jlong*)&INOUT);
    $input = jb;
%}

%typemap(directorin,descriptor="[J",warning="Need to provide int64_t *INOUT directorin typemap, int64_t array length is unknown") int64_t *INOUT
{
}

%typemap(typecheck) int64_t *INOUT = jlongArray;
%typemap(typecheck) int64_t &INOUT = jlongArray;

// The below typemap is added just to deallocate the memory for
// the java long array allocated above.  Unfortunately there doesn't seem to be a freearg
// equivalent for directorin typemaps.
%typemap(directorout) int64_t
{
    INOUT = ((int64_t *) jenv->GetLongArrayElements(jINOUT, 0))[0];
    (jenv)->DeleteLocalRef(jINOUT);
}

%typemap(jtype) int64_t "long";
%typemap(jstype) int64_t "long";
%typemap(jni) int64_t  "jlong"
%typemap(javain) int64_t  "$javainput"
%typemap(javadirectorin) int64_t "$jniinput"
%typemap(javadirectorout) int64_t "$javacall"
//%typemap(directorout) int64_t {}
%typemap(directorin,descriptor="J") int64_t {}
%typemap(javaout) int64_t {
    return $jnicall;
  }


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
 * All the required typemaps to allow a FutureResponse to be passed from Java to C++ (being serialized to a byte array in between)
 *******/
%typemap(jtype) const gravity::FutureResponse& "byte[]";
%typemap(jstype) const gravity::FutureResponse& "FutureResponse";
%typemap(jni)  const gravity::FutureResponse&  "jbyteArray"
%typemap(javain) const gravity::FutureResponse&  "$javainput.serializeToArray()"

%typemap(in) const gravity::FutureResponse&  {
    signed char* data = JCALL2(GetByteArrayElements, jenv, $input, NULL);
    int length = JCALL1(GetArrayLength, jenv, $input);
	$1 = new gravity::FutureResponse((void *)data, length);
	JCALL3(ReleaseByteArrayElements, jenv, $input, data, JNI_ABORT);
}

// this frees the memory allocated in the 'in' typemap above.
%typemap(freearg) const gravity::FutureResponse&  {
	delete $1;
}

/*****
 * Typemaps to handle passing a byte array from C++ into Java.  Conversion between
 * GDP and byte array is handled outside of this interface code (unlike the conversion
 * when passing GDP's from Java to C++).
 *****/
%typemap(directorin, descriptor="[B") char *BYTE {
    // length var is assumed to be passed into the function as well
    jbyteArray jb = (jenv)->NewByteArray(byteLength);
    (jenv)->SetByteArrayRegion(jb, 0, byteLength, (jbyte*)BYTE);
    $input = jb;
    // can't deallocate here because it hasn't been used yet.
//    (jenv)->DeleteLocalRef(jb);
}

// The below typemap is added just to deallocate the memory for
// the java byte array allocated above.  Unfortunately there doesn't seem to be a freearg
// equivalent for directorin typemaps.
%typemap(directorout) byte %{
    $result = ($1_ltype)$input;
    (jenv)->DeleteLocalRef(jBYTE);
%}

%typemap(javadirectorin) char *BYTE "$jniinput"
%typemap(javadirectorout) char *BYTE "$javacall"

/*****
 * Typemaps to handle passing a int array from C++ into Java.
 *****/
%typemap(directorin, descriptor="[I") int *INTEGER {
    // length var is assumed to be passed into the function as well
    jintArray jb = (jenv)->NewIntArray(intLength);
    (jenv)->SetIntArrayRegion(jb, 0, intLength, (jint*)INTEGER);
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
    (jenv)->DeleteLocalRef(jINTEGER);
%}

%typemap(javadirectorin) int *INTEGER "$jniinput"
%typemap(javadirectorout) int *INTEGER "$javacall"

%typemap(jni) int *INTEGER "jintArray"
%typemap(jtype) int *INTEGER "int[]"
%typemap(jstype) int *INTEGER "int[]"
%typemap(in) int *INTEGER {
  $1 = (int *) JCALL2(GetIntArrayElements, jenv, $input, 0);
}

%typemap(argout) int *INTEGER {
  JCALL3(ReleaseIntArrayElements, jenv, $input, (jint *) $1, 0);
}

%typemap(javain) int *INTEGER "$javainput"

/* Prevent default freearg typemap from being used */
%typemap(freearg) int *INTEGER ""



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
%typemap(out) shared_ptr<gravity::GravityDataProduct> {
  if ($1 != NULL)
  {
	  $result = JCALL1(NewByteArray, jenv, $1->getSize());
	  char *bytes = new char[$1->getSize()];
	  result->serializeToArray(bytes);
	  JCALL4(SetByteArrayRegion, jenv, $result, 0, $1->getSize(), (jbyte*)bytes);
	  delete bytes;
  }
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

/*****
 * typemaps to convert handle return of FutureResponse from java method so that it can travel across jni boundary as a byte[]
 ******/
%typemap(directorout) shared_ptr<gravity::FutureResponse> {
    signed char* data = JCALL2(GetByteArrayElements, jenv, $input, NULL);
    int length = JCALL1(GetArrayLength, jenv, $input);
	shared_ptr<gravity::FutureResponse> ret(new gravity::FutureResponse((void *)data, length));
	JCALL3(ReleaseByteArrayElements, jenv, $input, data, JNI_ABORT);
    $result = ret;
    (jenv)->DeleteLocalRef(jBYTE);
}
%typemap(out) shared_ptr<gravity::FutureResponse> {
  if ($1 != NULL)
  {
	  $result = JCALL1(NewByteArray, jenv, $1->getSize());
	  char *bytes = new char[$1->getSize()];
	  result->serializeToArray(bytes);
	  JCALL4(SetByteArrayRegion, jenv, $result, 0, $1->getSize(), (jbyte*)bytes);
	  delete bytes;
  }
}
%typemap(javadirectorout) shared_ptr<gravity::FutureResponse> "$javacall"
%typemap(jni) shared_ptr<gravity::FutureResponse> "jbyteArray"
%typemap(jtype) shared_ptr<gravity::FutureResponse> "byte[]"
%typemap(jstype) shared_ptr<gravity::FutureResponse> "byte[]"
%typemap(javaout) shared_ptr<gravity::FutureResponse> {
    return $jnicall;
  }

%typemap(javain) shared_ptr<gravity::FutureResponse> "$javainput"
%typemap(javadirectorin) shared_ptr<gravity::FutureResponse> "$jniinput"
%typemap(directorin, descriptor="[B") shared_ptr<gravity::FutureResponse> {}

/******
 * When gravity::GravityNode::request returns shared_ptr<GravityDataProduct> though, we want to
 * convert that to a GravityDataProduct on the java side of the JNI interface so that users
 * get a GDP rather than a byte[].
 ******/
%typemap(jstype) shared_ptr<gravity::GravityDataProduct> gravity::GravityNode::request "GravityDataProduct"
%typemap(javaout) shared_ptr<gravity::GravityDataProduct> gravity::GravityNode::request {
    byte[] data = $jnicall;
    if (data == null || data.length == 0)
        return null;
    return new GravityDataProduct(data);
  }

/******
 * When gravity::GravityNode::createFutureResponse returns shared_ptr<FutureResponse> though, we want to
 * convert that to a FutureResponse on the java side of the JNI interface so that users
 * get a FutureResponse rather than a byte[].
 ******/
%typemap(jstype) shared_ptr<gravity::FutureResponse> gravity::GravityNode::createFutureResponse "FutureResponse"
%typemap(javaout) shared_ptr<gravity::FutureResponse> gravity::GravityNode::createFutureResponse {
    byte[] data = $jnicall;
    if (data == null || data.length == 0)
        return null;
    return new FutureResponse(data);
  }

%include "modulecode.i"
%include "logger.i"
%include "gravitynode.i"
