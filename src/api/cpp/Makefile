CC=g++
THIRDPARTY_DIR=../../../ThirdParty
INIPARSE_DIR=$(THIRDPARTY_DIR)/iniparser
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
SQLLIB_DIR=$(THIRDPARTY_DIR)/cppdb-trunk
MYPROTO_DIR=../protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
LIB_NAME=libgravity.a

INCLUDES=-I$(GRAVLIB_DIR) $(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I$(SQLLIB_DIR)
CFLAGS=$(INCLUDES)

SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SRC))
DEPS=$(wildcard *.h $(MYPROTO_DIR)/*.h)

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,$(MYPROTO_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(MYPROTO_DIR)/%.cc,$(MYPROTO_DIR)/%.o,$(PROTO_CODE))

.PRECIOUS: %.pb.cc

all: $(LIB_NAME) 

%.pb.cc:%.proto
	@echo proto build $<
	@$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=$(MYPROTO_DIR) $<

%.pb.o:%.pb.cc
	@echo $(CC) proto build $<
	@$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cpp $(DEPS)
	@echo $(CC) $<
	@$(CC) -c -o $@ $< $(CFLAGS)

$(LIB_NAME): $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME)  
	@ar rcs $(LIB_NAME) $(OBJECTS) $(PROTO_OBJ)

clean:
	rm -rf *.o $(MYPROTO_DIR)/*.o $(MYPROTO_DIR)/*.h $(MYPROTO_DIR)/*.cc
