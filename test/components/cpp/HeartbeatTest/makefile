CC=g++
THIRDPARTY_DIR=../../../../ThirdParty
GRAVLIB_DIR=../../../../src/api/cpp
INIPARSE_DIR=$(THIRDPARTY_DIR)/iniparser
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
SQLLIB_DIR=$(THIRDPARTY_DIR)/cppdb-trunk
MYPROTO_DIR=$(GRAVLIB_DIR)/protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc

INCLUDES=-I$(GRAVLIB_DIR) -I$(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I$(SQLLIB_DIR)

LIBDIRS=-L$(GRAVLIB_DIR)/Windows -L$(ZMQ_DIR)/src/.libs -L$(PROTOBUF_DIR)/src/.libs -L$(INIPARSE_DIR)/
LIBS=-Wl,-Bstatic -lprotobuf -liniparser -Wl,-Bdynamic -lgravity -lzmq -lpthread -lwsock32


all: HeartbeatDummyListener HeartbeatDummy

HeartbeatDummyListener:
	g++ $(INCLUDES) $(LIBDIRS) HeartbeatDummyListener.cpp $(LIBS) -o HeartbeatDummyListener
	
HeartbeatDummy:
	g++ $(INCLUDES) $(LIBDIRS) HeartbeatDummy.cpp $(LIBS) -o HeartbeatDummy