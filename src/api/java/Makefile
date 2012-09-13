
include autosrcs.mk

CC=g++
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
PROTO_DIR=../protobufs
JAVASRC_DIR=src/java
CPPSRC_DIR=src/cpp
GRAVSRC_DIR=$(JAVASRC_DIR)/com/aphysci/gravity
PROTOGEN_DIR=$(GRAVSRC_DIR)/protobuf
SWIGGEN_DIR=$(GRAVSRC_DIR)/swig
PROTOC=$(PROTOBUF_DIR)/src/protoc
JAVA_BUILD_DIR=build
LIB_NAME=gravity.jar
JAVAINCLUDE_DIR=/usr/lib/jvm/java-7-oracle/include

INCLUDES=-I$(GRAVLIB_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I$(JAVAINCLUDE_DIR) -I$(JAVAINCLUDE_DIR)/linux -I../cpp -I.
CFLAGS=$(INCLUDES)

#CPP_SRC=gravity_wrap.cxx CPPGravitySubscriber.cpp
#CPP_OBJ=gravity_wrap.o CPPGravitySubscriber.o
CPP_OBJ=$(patsubst %.cpp,%.o,$(CPP_SRC))

OBJECTS=$(patsubst $(GRAVSRC_DIR)/%.java,$(JAVA_BUILD_DIR)/%.class,$(AUTO_SRCS))

PROTO_SRC=$(wildcard $(PROTO_DIR)/*PB.proto)
PROTO_CODE=$(patsubst $(PROTO_DIR)/%PB.proto,$(PROTOGEN_DIR)/%Container.java,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(PROTOGEN_DIR)%.java,$(JAVA_BUILD_DIR)%.class,$(PROTO_CODE))

.PRECIOUS: %.java $(PROTO_CODE)

all: $(LIB_NAME) libgravity_wrap.so

# Generate source from SWIG file and update AUTO_SRCS to include the generated .java files
# The changce to autosrcs.mk will cause the Makefile to be reloaded so the updated AUTO_SRCS will be seen.
autosrcs.mk: gravity.i
	swig -c++ -o $(CPPSRC_DIR)/gravity_wrap.cpp -outdir $(SWIGGEN_DIR) -package com.aphysci.gravity.swig -java $<
	echo "AUTO_SRCS=`echo $(SWIGGEN_DIR)/*.java $(GRAVSRC_DIR)/*.java`" > autosrcs.mk
	echo "CPP_SRC=`echo $(CPPSRC_DIR)/*.cpp`" >> autosrcs.mk

$(PROTOGEN_DIR)/%Container.java:$(PROTO_DIR)/%PB.proto
	@echo $<
	@$(PROTOC) --proto_path=$(PROTO_DIR) --java_out=$(JAVASRC_DIR) $<

$(JAVA_BUILD_DIR)/%.class:$(PROTOGEN_DIR)/%.java
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar:build:. $<

$(JAVA_BUILD_DIR)/%.class:$(GRAVSRC_DIR)/%.java
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar:build:. -sourcepath src/java $<

%.o: %.cpp
	$(CC) -c -fPIC -o $@ $< $(INCLUDES)

libgravity_wrap.so: $(CPP_OBJ) ../cpp/libgravity.a
	$(CC) -shared -o $@ $(CPP_OBJ) -Wl,-Bstatic -lgravity -lzmq -lprotobuf -L$(PROTOBUF_DIR)/src/.libs -L$(ZMQ_DIR)/src/.libs -L../cpp -lrt -Wl,-Bdynamic

$(LIB_NAME): autosrcs.mk $(PROTO_OBJ) $(OBJECTS)
	jar cf $@ -C $(JAVA_BUILD_DIR) .

test: all
	java -Djava.library.path=.:../cpp:$(PROTOBUF_DIR)/src/.libs:$(ZMQ_DIR)/src/.libs -cp build:$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar com.aphysci.gravity.GravityJavaTest

clean:
	rm -rf $(JAVASRC_DIR)/com/aphysci/gravity/swig/* $(PROTOGEN_DIR)/* $(JAVA_BUILD_DIR)/* $(CPPSRC_DIR)/gravity_wrap.* $(CPPSRC_DIR)/*.o *.jar *.so *.o autosrcs.mk 
