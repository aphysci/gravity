
include autosrcs.mk

CC=g++
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-jzmq
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
MYPROTO_DIR=../protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
JAVA_BUILD_DIR=build
LIB_NAME=gravity.jar

INCLUDES=-I$(GRAVLIB_DIR) $(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I/usr/lib/jvm/java-6-sun/include -I/usr/lib/jvm/java-6-sun/include/linux -I../cpp
CFLAGS=$(INCLUDES)

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
	javac -d $(JAVA_BUILD_DIR) -cp $(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar:. $<

gravity_wrap.o: gravity_wrap.cxx
	$(CC) -c -o $@ $< $(INCLUDES)

libgravity_wrap.so: gravity_wrap.o
	$(CC) -shared -o $@ $<

$(LIB_NAME): gravity.java $(OBJECTS) $(PROTO_OBJ)
	jar cf $@ -C $(JAVA_BUILD_DIR) .

clean:
	rm -rf Codes.java gravity.java gravityJNI.java GravityNode.java gravity_wrap.cxx  SWIGTYPE_p_GravityReturnCode.java gravity *.so *.o autosrcs.mk $(JAVA_BUILD_DIR)/* *.jar
