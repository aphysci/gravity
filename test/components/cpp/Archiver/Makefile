GRAVITY_DIR=../../../../src/api/cpp
PROTOBUF_DIR=../../../../ThirdParty/protobuf-2.4.1/src
ZMQ_DIR=../../../../zeromq-3.2.0

all:
	g++ -o DumbPub DummyPublisher.cpp -I$(GRAVITY_DIR) -L$(GRAVITY_DIR) -I$(PROTOBUF_DIR) -L$(ZMQ_DIR)/src/.libs -L$(PROTOBUF_DIR)/.libs -Wl,-Bstatic -lgravity -lprotobuf -lzmq -Wl,-Bdynamic -lpthread -lrt