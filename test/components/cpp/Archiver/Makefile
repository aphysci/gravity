GRAVITY_DIR=../../../../src/api/cpp

all:
	g++ -o DumbPub DummyPublisher.cpp -I$(GRAVITY_DIR) -L$(GRAVITY_DIR) -I$(PROTOBUF_INCLUDE_DIR) -L$(ZMQ_LIB_DIR) -L$(PROTOBUF_LIB_DIR) -Wl,-Bstatic -lgravity -lprotobuf -Wl,-Bdynamic -lzmq -lpthread -lwsock32
