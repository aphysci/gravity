CC=g++
THIRDPARTY_DIR=../../../ThirdParty
INIPARSE_DIR=$(THIRDPARTY_DIR)/iniparser
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
MYPROTO_DIR=../protobufs
PROTOC=$(PROTOBUF_DIR)/src/protoc
EZOPTION_PARSER=$(THIRDPARTY_DIR)/EZOptionParser
LIB_NAME=libgravity

INCLUDES=-I$(GRAVLIB_DIR) $(MYPROTO_DIR) -I$(ZMQ_DIR)/include -I$(PROTOBUF_DIR)/src -I$(INIPARSE_DIR)/src -I$(EZOPTION_PARSER)
CFLAGS=$(INCLUDES) -fPIC

SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SRC))
DEPS=$(wildcard *.h $(MYPROTO_DIR)/*.h)

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst %.cc,%.o,$(PROTO_CODE))

.PRECIOUS: %.pb.cc

all: $(LIB_NAME).a $(LIB_NAME).so

%.pb.cc:$(MYPROTO_DIR)/%.proto
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=. $<

%.pb.o:%.pb.cc
	@echo $(CC) proto build $<
	@$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cpp $(DEPS)
	@echo $(CC) $<
	@$(CC) -c -o $@ $< $(CFLAGS)

$(LIB_NAME).a: $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).a  
	@ar rcs $(LIB_NAME).a $(OBJECTS) $(PROTO_OBJ)

$(LIB_NAME).so: $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).so  
	@$(CC) -shared -o $(LIB_NAME).so $(OBJECTS) $(PROTO_OBJ)
#	$(CC) -shared -o $@ $< -Wl,-Bstatic -lzmq -lprotobuf -liniparser -L$(PROTOBUF_DIR)/src/.libs -L$(ZMQ_DIR)/src/.libs -L$(INIPARSE_DIR) -lrt -Wl,-Bdynamic

clean:
	rm -rf *.a *.so *.o *pb.h *.pb.cc
