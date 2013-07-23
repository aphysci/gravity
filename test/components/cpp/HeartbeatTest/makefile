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
