#** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#**
#** Gravity is free software; you can redistribute it and/or modify
#** it under the terms of the GNU Lesser General Public License as published by
#** the Free Software Foundation; either version 3 of the License, or
#** (at your option) any later version.
#**
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#** GNU Lesser General Public License for more details.
#**
#** You should have received a copy of the GNU Lesser General Public
#** License along with this program;
#** If not, see <http://www.gnu.org/licenses/>.
#**

CC=g++
GRAVLIB_DIR=../../../src/api/cpp
MYPROTO_DIR=../protobuf
PROTOC=$(PROTOBUF_DIR)/src/protoc
PROG_NAME=$(notdir $(shell pwd) )
COMPONENT_DIR=../../../src/components/cpp
SERVICEDIRECTORY_LOCATION=$(COMPONENT_DIR)/ServiceDirectory/

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

INCLUDES=-I$(GRAVLIB_DIR) -I$(MYPROTO_DIR) -I$(PROTOBUF_DIR)/src
CFLAGS=-L$(GRAVLIB_DIR) -L../../../src/keyvalue_parser/ -L$(PROTOBUF_LIB_DIR) -L$(ZMQ_LIB_DIR)
LIBS=-Wl,-Bstatic -lgravity -lprotobuf -lkeyvalue_parser $(OS_SPECIFIC_LIBS) $(ADDITIONAL_LIBS)

PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,$(MYPROTO_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(MYPROTO_DIR)/%.cc,$(MYPROTO_DIR)/%.o,$(PROTO_CODE))

# Don't delete generated .cc files so that eclipse (or any ide) is happy
.PRECIOUS: %.pb.cc

all: $(PROTO_OBJ) MultipleDataProductPublisher$(OS_EXE_EXT) MultipleDataProductSubscriber$(OS_EXE_EXT)

%.pb.cc:%.proto
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=$(MYPROTO_DIR) $<

%.pb.o:%.pb.cc
	$(CC) -c -o $@ $< $(INCLUDES)

#Compile each cpp file to a separate executable.
MultipleDataProductPublisher$(OS_EXE_EXT):MultipleDataProductPublisher.cpp $(PROTO_OBJ) $(GRAVLIB_DIR)/libgravity.a
	$(CC) -o $@ $(PROTO_OBJ) $< $(INCLUDES) $(CFLAGS) $(LIBS)

MultipleDataProductSubscriber$(OS_EXE_EXT):MultipleDataProductSubscriber.cpp $(PROTO_OBJ) $(GRAVLIB_DIR)/libgravity.a
	$(CC) -o $@ $(PROTO_OBJ) $< $(INCLUDES) $(CFLAGS) $(LIBS)

components: ServiceDirectory$(OS_EXE_EXT)

ServiceDirectory$(OS_EXE_EXT):
	cd $(SERVICEDIRECTORY_LOCATION); \
	make

clean:
	@rm -rf *.o $(MYPROTO_DIR)/*.o $(MYPROTO_DIR)/*pb.h $(MYPROTO_DIR)/*pb.cc MultipleDataProductSubscriber$(OS_EXE_EXT) MultipleDataProductPublisher$(OS_EXE_EXT)

run: components MultipleDataProductPublisher$(OS_EXE_EXT) MultipleDataProductSubscriber$(OS_EXE_EXT)
	./run.sh
