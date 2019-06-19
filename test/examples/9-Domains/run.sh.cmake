#!/bin/bash
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

# Tool-related substitution variables
PROTOBUF_LIB_DIR=${PROTOBUF_LIBRARY_PATH}
ZMQ_LIB_DIR=${ZeroMQ_LIBRARY_PATH}

export PATH=$PATH:${ServiceDirectory_BINARY_DIR}
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${libgravity_BINARY_DIR}:${libkeyvalue_parser_BINARY_DIR}:$ZMQ_LIB_DIR:$PROTOBUF_LIB_DIR

cleanup()
# run if user hits control-c
{ 
  kill -9 $SDPID
  kill -9 $PUBPID
  kill -9 $SUBPID
  exit $?
}

# trap keyboard interrupt (control-c)
trap cleanup SIGINT

./ProtobufDataProductPublisher &
PUBPID=$!

./ProtobufDataProductSubscriber &
SUBPID=$!

sleep 10

ServiceDirectory &
SDPID=$!

sleep 10

kill $SDPID $PUBPID
echo Killed ServiceDirectory and ProtobufDataProductPublisher
sleep 10

echo Restarting ServiceDirectory and ProtobufDataProductPublisher
./ProtobufDataProductPublisher &
PUBPID=$!
ServiceDirectory &
SDPID=$!

sleep 30

cleanup

