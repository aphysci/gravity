# (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#
# Gravity is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program;
# If not, see <http://www.gnu.org/licenses/>.



#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_CONFIG_MACRO_DIRS([m4])
AC_PREREQ([2.68])
AC_INIT([Gravity], [1.0], [gravity@aphysci.com])
AC_CONFIG_SRCDIR([test/GravityTest.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_MAKE_SET
AC_PROG_YACC

# Checks for libraries.
AC_CHECK_LIB([gravity], [main])
AC_CHECK_LIB([iniparser], [main])
AC_CHECK_LIB([keyvalue_parser], [main])
AC_CHECK_LIB([protobuf], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([rt], [main])
AC_CHECK_LIB([wsock32], [main])
AC_CHECK_LIB([zmq], [main])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h inttypes.h libintl.h malloc.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Allow user to disable Gravity's java capability
AC_ARG_WITH([java],
  [AS_HELP_STRING([--without-java],
    [Disable Gravity's support for java.])],
  [],
  [WITHOUT_JAVA=no])

AS_IF([test "x$WITHOUT_JAVA" != xno],[WITHOUT_JAVA=yes])
AC_SUBST([WITHOUT_JAVA])

# Check for bison program
AS_IF([test "x$ac_cv_prog_YACC" != "xbison -y"],[
       AC_MSG_ERROR([Bison is required to compile Gravity. Make sure it is installed and in the PATH.])
       ])

# Check for flex program
AS_IF([test "x$ac_cv_prog_LEX" != "xflex"],[
       AC_MSG_ERROR([Flex is required to compile Gravity. Make sure it is installed and in the PATH.])
       ])

if test $WITHOUT_JAVA == no
then
   # Check for swig program
   AC_CHECK_PROG(SWIGPROG, swig, swig, MISSING)
   if test $SWIGPROG = "MISSING"; then
     AC_MSG_ERROR([Swig is required to compile Gravity. Make sure it is installed and in the PATH.])
   fi

   # Check for JNI headers
   JNI_CPPFLAGS=""
   if test $JAVA_HOME != ""
   then
     for dir in `find $JAVA_HOME/include -follow -type d`
     do
       JNI_CPPFLAGS="$JNI_CPPFLAGS -I$dir"
     done
   fi
   cppflags_bak=$CPPFLAGS
   CPPFLAGS="$CPPFLAGS $JNI_CPPFLAGS"

   AC_CHECK_HEADERS([jni.h], [], AC_MSG_ERROR([Java header jni.h is not found. Please ensure \$JAVA_HOME is defined correctly or the Java Development Kit is installed properly.]))
   CPPFLAGS=$cppflags_bak
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime gettimeofday memset socket strdup])

# Check for protobuf library
AC_ARG_WITH([protobuf-libdir],
    [AS_HELP_STRING([--with-protobuf-libdir=LIB_DIR],
        [location of the protocol buffers libraries, defaults to /usr/local/lib])],
    [PROTOBUF_LIBDIR="$withval"],
    [PROTOBUF_LIBDIR='/usr/local/lib'])
AC_SUBST([PROTOBUF_LIBDIR])

# Check for protobuf include path
AC_ARG_WITH([protobuf-incdir],
    [AS_HELP_STRING([--with-protobuf-incdir=INC_DIR],
        [location of the protocol buffers include files, defaults to /usr/local/include])],
    [PROTOBUF_INCDIR="$withval"],
    [PROTOBUF_INCDIR='/usr/local/include'])
AC_SUBST([PROTOBUF_INCDIR])
CPPFLAGS="$CPPFLAGS -I$PROTOBUF_INCDIR"
LDFLAGS="$LDFLAGS -L$PROTOBUF_LIBDIR"

# Check for zeromq library
AC_ARG_WITH([zeromq-libdir],
    [AS_HELP_STRING([--with-zeromq-libdir=LIB_DIR],
        [location of the zeromq libraries, defaults to /usr/local/lib])],
    [ZEROMQ_LIBDIR="$withval"],
    [ZEROMQ_LIBDIR='/usr/local/lib'])
AC_SUBST([ZEROMQ_LIBDIR])

LDFLAGS="$LDFLAGS -L$ZEROMQ_LIBDIR"

# Check for zeromq include path
AC_ARG_WITH([zeromq-incdir],
    [AS_HELP_STRING([--with-zeromq-incdir=INC_DIR],
        [location of the zeromq includes, defaults to /usr/local/include])],
    [ZEROMQ_INCDIR="$withval"],
    [ZEROMQ_INCDIR='/usr/local/include'])
AC_SUBST([ZEROMQ_INCDIR])
CPPFLAGS="$CPPFLAGS -I$ZEROMQ_INCDIR"

# Check for version of zeromq library - need at least 3.x version
# But macro does not work during cross-compile, so skip test in that case
if test $cross_compiling != "yes"
then
    AC_CHECK_FILES([$ZEROMQ_INCDIR/zmq.h],,[AC_MSG_ERROR([$ZEROMQ_INCDIR/zmq.h not found. Check the --with-zeromq-incdir option.])])
fi

MAJOR=`egrep '^#define +ZMQ_VERSION_MAJOR +[0-9]+$' $ZEROMQ_INCDIR/zmq.h`
MINOR=`egrep '^#define +ZMQ_VERSION_MINOR +[0-9]+$' $ZEROMQ_INCDIR/zmq.h`
PATCH=`egrep '^#define +ZMQ_VERSION_PATCH +[0-9]+$' $ZEROMQ_INCDIR/zmq.h`

MAJOR=`echo $MAJOR | awk '{ print $3 }'`
MINOR=`echo $MINOR | awk '{ print $3 }'`
PATCH=`echo $PATCH | awk '{ print $3 }'`

if test $MAJOR == "2"
then
  AC_MSG_ERROR([The Gravity package requires at least version 3.x of ZeroMQ.])
fi

AC_ARG_WITH([protoc],
    [AS_HELP_STRING([--with-protoc=PATH],
        [location of the protoc protocol buffer compiler binary, defaults to protoc])],
    [PROTOC="$withval"],
    [PROTOC='protoc'])
AC_SUBST([PROTOC])

# Check for protoc program
# But macro does not work during cross-compile, so skip test in that case
if test $cross_compiling != "yes"
then
    AC_CHECK_FILES([$PROTOC],,[AC_MSG_ERROR([$PROTOC not found. Check the --with-protoc option.])])
fi


# Check for Boost Libraries
BOOST_REQUIRE([],[AC_MSG_ERROR(Unable to find the Boost C++ Libraries.  Please ensure the Boost C++ Libraries are installed.)])

PROTOBUF_VER=`pkg-config --modversion protobuf`

if test $WITHOUT_JAVA == no
then
   AC_ARG_VAR(JAVAPROTOBUF_DIR, [Location of the Java Protocol Buffers runtime library.  Default = /opt/protobuf-<protobuf_version>/java/target/protobuf-java-<protobuf_version>.jar])
   AS_IF([test "x$JAVAPROTOBUF_DIR" = x], [JAVAPROTOBUF_DIR="/opt/protobuf-$PROTOBUF_VER/java/target/protobuf-java-$PROTOBUF_VER.jar"])
   # macro does not work during cross-compile, so skip test in that case
   if test $cross_compiling != "yes"
   then
      AC_CHECK_FILES([$JAVAPROTOBUF_DIR],,[AC_MSG_ERROR([JAVAPROTOBUF_DIR not found. Type 'configure --help' for details.])])
   fi

   AC_ARG_VAR(GUAVAJAR_DIR, [Location of the Guava Google Core Libraries.  Default = /opt/guava-13.0.1/guava-13.0.1.jar])
   AS_IF([test "x$GUAVAJAR_DIR" = x], [GUAVAJAR_DIR="/opt/guava-13.0.1/guava-13.0.1.jar"])
   # macro does not work during cross-compile, so skip test in that case
   if test $cross_compiling != "yes"
   then
      AC_CHECK_FILES([$GUAVAJAR_DIR],,[AC_MSG_ERROR([GUAVAJAR_DIR not found. Type 'configure --help' for details.])])
   fi
fi

CPPFLAGS="$CPPFLAGS -Wall"

AC_CONFIG_FILES([Makefile
                 src/api/MATLAB/Makefile
                 src/api/cpp/Makefile
                 src/components/cpp/Makefile
                 src/components/cpp/Archiver/Makefile
                 src/components/cpp/ConfigServer/Makefile
                 src/components/cpp/LogRecorder/Makefile
                 src/components/cpp/Playback/Makefile
                 src/components/cpp/ServiceDirectory/Makefile
                 src/keyvalue_parser/Makefile
                 test/components/cpp/Makefile
                 test/components/cpp/ServiceDirectory/Makefile
                 test/components/cpp/ServiceDirectoryReregister/Makefile
                 src/api/java/cpp_makefile
                 src/api/java/java_makefile
                 test/api/java/Makefile
                 test/components/cpp/ServiceDirectory/test.sh
                 test/components/cpp/ServiceDirectoryReregister/test.sh
                 test/api/java/test.sh
                 test/examples/1-BasicDataProduct/Makefile
                 test/examples/1-BasicDataProduct/run.sh
                 test/examples/2-ProtobufDataProduct/Makefile
                 test/examples/2-ProtobufDataProduct/run.sh
                 test/examples/3-MultipleDataProduct/Makefile
                 test/examples/3-MultipleDataProduct/run.sh
                 test/examples/4-BasicService/Makefile
                 test/examples/4-BasicService/run.sh
                 test/examples/5-MiscFunctionality/Makefile
                 test/examples/5-MiscFunctionality/run.sh
                 test/examples/6-JavaProtobufDataProduct/Makefile
                 test/examples/6-JavaProtobufDataProduct/run.sh
                 test/examples/7-JavaBasicService/Makefile
                 test/examples/7-JavaBasicService/run.sh
                 test/examples/8-ConfigFile/Makefile
                 test/examples/8-ConfigFile/run.sh
                 test/examples/9-Domains/Makefile
                 test/examples/9-Domains/run.sh
                 test/examples/10-Archiving/Makefile
                 test/examples/10-Archiving/run.sh])

AC_OUTPUT

echo \
"-------------------------------------------------

 ${PACKAGE_NAME}: Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 Compiler: '${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS}'

 Now type 'make <target>'
   where the optional <target> is:
     clean         - remove binaries and objects
     all           - build all functionality
     install       - install required binaries
     test          - test functionality

--------------------------------------------------"

