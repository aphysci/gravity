CC=g++
THIRDPARTY_DIR=../../../ThirdParty
INIPARSE_DIR=$(THIRDPARTY_DIR)/iniparser
SQLLIB_DIR=$(THIRDPARTY_DIR)/cppdb-trunk
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
EZOPTION_PARSER_DIR=$(THIRDPARTY_DIR)/EZOptionParser
GRAVLIB_DIR=../../../src/api/cpp/
MYPROTO_DIR=protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
PROG_NAME=$(notdir $(shell pwd) )
COMPONENT_DIR=../../../src/components/cpp
SERVICEDIRECTORY_LOCATION=$(COMPONENT_DIR)/ServiceDirectory/
CONFIGSERVER_LOCATION=$(COMPONENT_DIR)/ConfigServer/

#OS Specific
SYSTEM:=$(strip $(shell uname -s))
#SYSTEM:=LINUX
#SYSTEM=MINGW32_NT-6.1
#WARNING: DON'T PUT SPACES OR QUOTES IN COMPARISON STRING!!!
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
	OS_SPECIFIC_LIBS = -Wl,-Bdynamic -lzmq -lwsock32 -lpthread
	OS_EXE_EXT=.exe
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
	OS_SPECIFIC_LIBS = -lzmq -Wl,-Bdynamic -lrt
	OS_EXE_EXT=
linux: all;
else
ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

INCLUDES=-I$(GRAVLIB_DIR) -I$(ZMQ_DIR)/include -I$(MYPROTO_DIR) -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I$(SQLLIB_DIR) -I$(EZOPTION_PARSER_DIR)
CFLAGS=-L$(GRAVLIB_DIR)/Windows -L$(ZMQ_DIR)/src/.libs -L$(PROTOBUF_DIR)/src/.libs -L$(INIPARSE_DIR) -L$(SQLLIB_DIR)
LIBS=-lgravity -Wl,-Bstatic -lprotobuf -liniparser $(OS_SPECIFIC_LIBS) $(ADDITIONAL_LIBS)

SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SRC))
DEPS=$(wildcard *.h $(MYPROTO_DIR)/*.h)
OUTPUT=$(patsubst %.cpp,%$(OS_EXE_EXT),$(SRC))

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,$(MYPROTO_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(MYPROTO_DIR)/%.cc,$(MYPROTO_DIR)/%.o,$(PROTO_CODE))

all: BasicDataProductPublisher$(OS_EXE_EXT) BasicDataProductSubscriber$(OS_EXE_EXT)

%.pb.cc:%.proto
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=$(MYPROTO_DIR) $<

%.pb.o:%.pb.cc
	$(CC) -c -o $@ $< $(INCLUDES)

#Compile each cpp file to a separate executable.  
BasicDataProductPublisher$(OS_EXE_EXT):BasicDataProductPublisher.cpp $(DEPS)
	$(CC) -o $@ $< $(INCLUDES) $(CFLAGS) $(LIBS)

BasicDataProductSubscriber$(OS_EXE_EXT):BasicDataProductSubscriber.cpp $(DEPS)
	$(CC) -o $@ $< $(INCLUDES) $(CFLAGS) $(LIBS)
	
components: ServiceDirectory$(OS_EXE_EXT)

ServiceDirectory$(OS_EXE_EXT):
	cd $(SERVICEDIRECTORY_LOCATION); \
	make

ConfigService$(OS_EXE_EXT):
	cd $(CONFIGSERVER_LOCATION); \
	make
	
clean:
	@rm -rf *.o $(MYPROTO_DIR)/*.o $(MYPROTO_DIR)/*.h $(MYPROTO_DIR)/*.cc $(PROG_NAME)

run: components BasicDataProductPublisher$(OS_EXE_EXT) BasicDataProductSubscriber$(OS_EXE_EXT)
	$(SERVICEDIRECTORY_LOCATION)/ServiceDirectory$(OS_EXE_EXT) &
	BasicDataProductPublisher$(OS_EXE_EXT) &
	BasicDataProductSubscriber$(OS_EXE_EXT) &