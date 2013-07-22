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
GRAVLIB_DIR=../../../src/api/cpp/
COMPONENT_DIR=../../../src/components/cpp
SERVICEDIRECTORY_LOCATION=$(COMPONENT_DIR)/ServiceDirectory

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

INCLUDES=-I$(GRAVLIB_DIR) -I$(PROTOBUF_DIR)/src
CFLAGS=-L$(GRAVLIB_DIR) -L../../../src/keyvalue_parser/ -L$(PROTOBUF_LIB_DIR) -L$(ZMQ_LIB_DIR)
LIBS=-Wl,-Bstatic -lgravity -lkeyvalue_parser -lprotobuf $(OS_SPECIFIC_LIBS) 

all: BasicDataProductPublisher$(OS_EXE_EXT) BasicDataProductSubscriber$(OS_EXE_EXT)

#Compile each cpp file to a separate executable.  
BasicDataProductPublisher$(OS_EXE_EXT):BasicDataProductPublisher.cpp
	$(CC) -o $@ $< $(INCLUDES) $(CFLAGS) $(LIBS)

BasicDataProductSubscriber$(OS_EXE_EXT):BasicDataProductSubscriber.cpp
	$(CC) -o $@ $< $(INCLUDES) $(CFLAGS) $(LIBS)
	
components: ServiceDirectory$(OS_EXE_EXT)

ServiceDirectory$(OS_EXE_EXT):
	@cd $(SERVICEDIRECTORY_LOCATION); \
	make

clean:
	@rm -rf *.o *.exe BasicDataProductPublisher BasicDataProductSubscriber

run: components BasicDataProductPublisher$(OS_EXE_EXT) BasicDataProductSubscriber$(OS_EXE_EXT)
	./run.sh
