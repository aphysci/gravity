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


export PATH=$PATH:$PROTOBUF_INCLUDE_DIR:../../../src/api/java:../../../src/components/cpp/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ZMQ_LIB_DIR:../../../src/api/cpp:../../../src/keyvalue_parser:$ZMQ_LIB_DIR:$PROTOBUF_LIB_DIR

ServiceDirectory &
SDPID=$!
java -cp build:$JAVAPROTOBUF_DIR:../../../src/api/java/gravity.jar -Djava.library.path=../../../src/api/java:$PROTOBUF_INCLUDE_DIR:$ZMQ_LIB_DIR JavaProtobufDataProductPublisher &
JDPPID=$!

# sleep for a second to give the publisher a chance to register its data product
sleep 1

java -cp build:$JAVAPROTOBUF_DIR:../../../src/api/java/gravity.jar -Djava.library.path=../../../src/api/java:$PROTOBUF_INCLUDE_DIR:$ZMQ_LIB_DIR JavaProtobufDataProductSubscriber

kill $SDPID
kill $JDPPID
