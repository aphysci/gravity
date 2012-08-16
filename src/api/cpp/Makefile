
CC=g++
PROTOC=../../../ThirdParty/protobuf-2.4.1/src/protoc
LIB_NAME=libgravity.a
PROTO_DIR=protobufs
CFLAGS=-I$(PROTO_DIR)
SRC_DIR=.
SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SRC))

PROTO_SRC=$(wildcard $(PROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(PROTO_DIR)/%.proto,$(PROTO_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(PROTO_DIR)/%.cc,$(PROTO_DIR)/%.o,$(PROTO_CODE))

DEPS=$(wildcard *.h $(PROTO_DIR)/*.h)

all: $(LIB_NAME) 

%.pb.cc:%.proto
	$(PROTOC) --proto_path=$(PROTO_DIR) --cpp_out=$(PROTO_DIR) $<

%.pb.o: %.pb.cc
	$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(LIB_NAME): $(PROTO_OBJ) $(OBJECTS)
	ar rcs $(LIB_NAME) $(OBJECTS) $(PROTO_OBJ)

clean:
	@rm -rf *.o $(PROTO_DIR)/*.o $(PROTO_DIR)/*.h $(PROTO_DIR)/*.cc
