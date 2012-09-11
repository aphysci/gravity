#Locations
MYPROTO_DIR=../protobufs
THIRDPARTY_DIR=../../../ThirdParty

#Libraries
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
PROTOBUF_INCLUDE_DIR=$(PROTOBUF_DIR)/src
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
ZMQ_INCLUDE_DIR=$(ZMQ_DIR)/include
INIPARSE_DIR=$(THIRDPARTY_DIR)/iniparser
INIPARSE_INCLUDE_DIR=$(THIRDPARTY_DIR)/iniparser/src
EZOPTION_PARSER_DIR=$(THIRDPARTY_DIR)/EZOptionParser

#Tools
PROTOC=$(PROTOBUF_DIR)/src/protoc
CC=g++

#Naming
LIB_NAME=libgravity

#OS Specific
SYSTEM = $(shell uname -s)
ifeq ( $(findstring 'MINGW_NT',$(SYSTEM)), 'MINGW32_NT')
	LIB_EXT = dll
	#Not including rt and zmq
	OS_SPECIFIC_LIBS = -Wl,-Bstatic -lprotobuf -liniparser -Wl,-Bdynamic
else ifeq ($(findstring 'LINUX',$(SYSTEM)), 'LINUX')
	LIB_EXT = so
	OS_SPECIFIC_LIBS = -Wl,-Bstatic -lzmq -lprotobuf -liniparser -lrt -Wl,-Bdynamic
	OS_SPECIFIC_FLAGS = -fpic
endif 

#Compiler Flags
INCLUDES=-I$(ZMQ_INCLUDE_DIR) -I$(PROTOBUF_INCLUDE_DIR) -I$(INIPARSE_INCLUDE_DIR) -I$(EZOPTION_PARSER_DIR)
LIBDIRS=-L$(PROTOBUF_DIR)/src/.libs -L$(ZMQ_DIR)/src/.libs -L$(INIPARSE_DIR) 
LIBS=$(OS_SPECIFIC_LIBS)
CFLAGS=$(INCLUDES) $(OS_SPECIFIC_FLAGS)

#Source/Object/Dependancies
SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SRC))
DEPS=$(wildcard *.h $(MYPROTO_DIR)/*.h)

#Protobuf Source/Generated c++/Objects
PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst %.cc,%.o,$(PROTO_CODE))

.PRECIOUS: %.pb.cc

all: $(LIB_NAME).a $(LIB_NAME).$(LIB_EXT)

%.pb.cc:$(MYPROTO_DIR)/%.proto
	@echo protoc $<
	@$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=. $<

%.pb.o:%.pb.cc
	@echo $(CC) proto build $<
	$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cpp $(DEPS)
	@echo $(CC) $<
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIB_NAME).a: $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).a  
	@ar rcs $(LIB_NAME).a $(OBJECTS) $(PROTO_OBJ)

$(LIB_NAME).$(LIB_EXT): $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).$(LIB_EXT)  
#	@$(CC) -shared -o $@ $(OBJECTS) $(PROTO_OBJ)
	$(CC) -shared -o $@ $< $(LIBDIRS) $(LIBS)

clean:
	rm -rf *.a *.so *.o *pb.h *.pb.cc
