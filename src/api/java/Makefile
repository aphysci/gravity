
include autosrcs.mk

CC=g++
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-jzmq
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
MYPROTO_DIR=../protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
LIB_NAME=gravity.jar

INCLUDES=-I$(GRAVLIB_DIR) $(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I/usr/lib/jvm/java-6-sun/include -I/usr/lib/jvm/java-6-sun/include/linux -I../cpp
CFLAGS=$(INCLUDES)

OBJECTS=$(patsubst %.java,%.class,$(AUTO_SRCS))

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*PB.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%PB.proto,gravity/%.java,$(PROTO_SRC))
PROTO_OBJ=$(patsubst %.java,%.class,$(PROTO_CODE))

.PRECIOUS: %.pb.java

all: $(LIB_NAME) libgravity_wrap.so

#gravity.java: gravity.i
autosrcs.mk: gravity.i
	swig -c++ -java $<
	echo "AUTO_SRCS=`echo \`ls -R | grep java$ \``" > autosrcs.mk

#%.pb.java:$(patsubst gravity/%.pb.java,$(MYPROTO_DIR)/%PB.proto,)
gravity/%.java:$(MYPROTO_DIR)/%PB.proto
	echo $@
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --java_out=. $<

#%.pb.class:%.pb.java
#	javac $<

%.class:%.java
	javac $<

gravity_wrap.o: gravity_wrap.cxx
	$(CC) -c -o $@ $< $(INCLUDES)

libgravity_wrap.so: gravity_wrap.o
	$(CC) -shared -o $@ $<

$(LIB_NAME): gravity.java $(OBJECTS) $(PROTO_OBJ)
#	@echo building $(LIB_NAME)  

clean:
	rm -rf Codes.java gravity.java gravityJNI.java GravityNode.java gravity_wrap.cxx  SWIGTYPE_p_GravityReturnCode.java gravity *.so *.o autosrcs.mk
	find . -name "*.class" -exec rm {} \;
