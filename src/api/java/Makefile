
include autosrcs.mk

CC=g++
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
INIPARSER_DIR=$(THIRDPARTY_DIR)/iniparser
PROTO_DIR=../protobufs
JAVASRC_DIR=src/java
GRAVLIB_DIR=../cpp
CPPSRC_DIR=src/cpp
SWIGSRC_DIR=src/swig
GRAVSRC_DIR=$(JAVASRC_DIR)/com/aphysci/gravity
PROTOGEN_DIR=$(GRAVSRC_DIR)/protobuf
SWIGGEN_DIR=$(GRAVSRC_DIR)/swig
PROTOC=$(PROTOBUF_DIR)/src/protoc
JAVA_BUILD_DIR=build
LIB_NAME=gravity.jar
JAVAINCLUDE_DIR="$(JAVA_HOME)/include"

#CPP_SRC=gravity_wrap.cxx CPPGravitySubscriber.cpp
#CPP_OBJ=gravity_wrap.o CPPGravitySubscriber.o
CPP_OBJ=$(patsubst %.cpp,%.o,$(CPP_SRC))

OBJECTS=$(patsubst $(GRAVSRC_DIR)/%.java,$(JAVA_BUILD_DIR)/%.class,$(AUTO_SRCS))

PROTO_SRC=$(PROTO_DIR)/GravityDataProductPB.proto
#PROTO_SRC=$(wildcard $(PROTO_DIR)/*PB.proto)
PROTO_CODE=$(patsubst $(PROTO_DIR)/%PB.proto,$(PROTOGEN_DIR)/%Container.java,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(PROTOGEN_DIR)%.java,$(JAVA_BUILD_DIR)%.class,$(PROTO_CODE))

SYSTEM:=$(strip $(shell uname -s))
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
	OS_SPECIFIC_LIBS = -Wl,-Bdynamic -lzmq -lwsock32 -lpthread
	CLASSPATH="$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar;build;."
	OS_SPECIFIC_INCS=-I$(JAVAINCLUDE_DIR)/win32
	OS_SPECIFIC_LIBFLAGS=-D_JNI_IMPLEMENTATION_ -Wl,--kill-at
	LIB_EXT=dll
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
	OS_SPECIFIC_LIBS = -lrt -Wl,-Bdynamic -lzmq 
	OS_SPECIFIC_INCS=-I$(JAVAINCLUDE_DIR)/linux
	CLASSPATH=$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar:build:.
	LIB_EXT=so
linux: all;
else
	ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

INCLUDES=-I$(GRAVLIB_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSER_DIR)/src -I$(JAVAINCLUDE_DIR) -I$(GRAVLIB_DIR) -I. $(OS_SPECIFIC_INCS)
CFLAGS=-L$(GRAVLIB_DIR) -L$(ZMQ_DIR)/src/.libs -L$(PROTOBUF_DIR)/src/.libs -L$(INIPARSER_DIR)
LIBS=-Wl,-Bstatic -lgravity -lprotobuf -liniparser $(OS_SPECIFIC_LIBS)

.PRECIOUS: %.java $(PROTO_CODE)

all: $(LIB_NAME) libgravity_wrap.$(LIB_EXT)

# Generate source from SWIG files and update AUTO_SRCS to include the generated .java files
# The change to autosrcs.mk will cause the Makefile to be reloaded so the updated AUTO_SRCS will be seen.
autosrcs.mk: $(SWIGSRC_DIR)/gravity.i $(wildcard $(SWIGSRC_DIR)/*.i)
	swig -c++ -o $(CPPSRC_DIR)/gravity_wrap.cpp -outdir $(SWIGGEN_DIR) -package com.aphysci.gravity.swig -java $<
	echo "AUTO_SRCS=`echo $(SWIGGEN_DIR)/*.java $(GRAVSRC_DIR)/*.java`" > autosrcs.mk
	echo "CPP_SRC=`echo $(CPPSRC_DIR)/*.cpp`" >> autosrcs.mk

$(PROTOGEN_DIR)/%Container.java:$(PROTO_DIR)/%PB.proto
	@echo $<
	@$(PROTOC) --proto_path=$(PROTO_DIR) --java_out=$(JAVASRC_DIR) $<

$(JAVA_BUILD_DIR)/%.class:$(PROTOGEN_DIR)/%.java
	@echo $<
	javac -d $(JAVA_BUILD_DIR) -cp $(CLASSPATH) $<

$(JAVA_BUILD_DIR)/%.class:$(GRAVSRC_DIR)/%.java
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(CLASSPATH) -sourcepath src/java $<

%.o: %.cpp
	$(CC) -c -fPIC -o $@ $< $(INCLUDES)

libgravity_wrap.$(LIB_EXT): $(CPP_OBJ) $(GRAVLIB_DIR)/libgravity.a
	$(CC) $(OS_SPECIFIC_LIBFLAGS) -shared -o $@ $(CPP_OBJ) $(CFLAGS) $(LIBS)

$(LIB_NAME): autosrcs.mk $(PROTO_OBJ) $(OBJECTS)
	jar cf $@ -C $(JAVA_BUILD_DIR) .

clean:
	rm -rf $(JAVASRC_DIR)/com/aphysci/gravity/swig/* $(PROTOGEN_DIR)/* $(JAVA_BUILD_DIR)/* $(CPPSRC_DIR)/gravity_wrap.* $(CPPSRC_DIR)/*.o *.jar *.so *.dll *.o autosrcs.mk 
