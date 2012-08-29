CC=g++
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-jzmq
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
MYPROTO_DIR=../protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
LIB_NAME=gravity.jar

INCLUDES=-I$(GRAVLIB_DIR) $(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I$(SQLLIB_DIR)
CFLAGS=$(INCLUDES)

SRC=$(wildcard *.java)
OBJECTS=$(patsubst %.java,%.class,$(SRC))

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,%.pb.java,$(PROTO_SRC))
PROTO_OBJ=$(patsubst %.cc,%.class,$(PROTO_CODE))

.PRECIOUS: %.pb.java

all: $(LIB_NAME) 

%.pb.java:$(MYPROTO_DIR)/%.proto
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --java_out=. $<

%.pb.class:%.pb.java
	@echo $(CC) proto build $<
	@$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cpp $(DEPS)
	@echo $(CC) $<
	@$(CC) -c -o $@ $< $(CFLAGS)

$(LIB_NAME): $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME)  
	@ar rcs $(LIB_NAME) $(OBJECTS) $(PROTO_OBJ)

clean:
	rm -rf *.o *pb.h *.pb.cc
