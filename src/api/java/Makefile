
include autosrcs.mk

CC=g++
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-jzmq
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
MYPROTO_DIR=../protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
JAVA_BUILD_DIR=build
LIB_NAME=gravity.jar

INCLUDES=-I$(GRAVLIB_DIR) $(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I/usr/lib/jvm/java-6-sun/include -I/usr/lib/jvm/java-6-sun/include/linux -I../cpp -I.
CFLAGS=$(INCLUDES)

CPP_SRC=gravity_wrap.cxx CPPGravitySubscriber.cpp
CPP_OBJ=gravity_wrap.o CPPGravitySubscriber.o

OBJECTS=$(patsubst %.java,%.class,$(AUTO_SRCS))

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*PB.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%PB.proto,gravity/%.java,$(PROTO_SRC))
PROTO_OBJ=$(patsubst %.java,%.class,$(PROTO_CODE))

.PRECIOUS: %.java $(PROTO_CODE)

all: $(LIB_NAME) libgravity_wrap.so

# Generate source from SWIG file and update AUTO_SRCS to include the generated .java files
# The changce to autosrcs.mk will cause the Makefile to be reloaded so the updated AUTO_SRCS will be seen.
autosrcs.mk: gravity.i
	swig -c++ -java $<
	echo "AUTO_SRCS=`echo *.java`" > autosrcs.mk

gravity/%.java:$(MYPROTO_DIR)/%PB.proto
	echo $@
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --java_out=. $<

%.class:%.java
	javac -d $(JAVA_BUILD_DIR) -cp $(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar:build:. $<

%.o: %.cpp
	$(CC) -c -o $@ $< $(INCLUDES)

%.o: %.cxx
	$(CC) -c -o $@ $< $(INCLUDES)

libgravity_wrap.so: $(CPP_OBJ) ../cpp/libgravity.a
	$(CC) -shared -o $@ $(CPP_OBJ) -Wl,-Bstatic -lgravity -lzmq -lprotobuf -L../../../ThirdParty/protobuf-2.4.1/src/.libs -L../../../ThirdParty/zeromq-3.2.0/src/.libs -L../cpp -lrt -Wl,-Bdynamic

$(LIB_NAME): autosrcs.mk gravity.java $(PROTO_OBJ) $(OBJECTS)
	jar cf $@ -C $(JAVA_BUILD_DIR) .

run:
	java -Djava.library.path=.:../cpp:../../../ThirdParty/protobuf-2.4.1/src/.libs:../../../ThirdParty/zeromq-3.2.0/src/.libs -cp build:$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar GravityJavaTest

clean:
	rm -rf Codes.java gravity.java gravityJNI.java GravityNode.java gravity_wrap.cxx  SWIGTYPE_p_GravityReturnCode.java gravity *.so *.o autosrcs.mk $(JAVA_BUILD_DIR)/* *.jar SWIGTYPE*.java GravityReturnCode.java NativeGravitySubscriber.java CPPGravitySubscriber.java
