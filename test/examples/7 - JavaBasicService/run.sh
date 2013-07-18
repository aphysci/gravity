#!/bin/bash

export PATH=$PATH:$PROTOBUF_INCLUDE_DIR:../../../src/api/java:../../../src/components/cpp/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ZMQ_LIB_DIR:../../../src/api/cpp:../../../src/keyvalue_parser

ServiceDirectory &
SDPID=$!
java -cp build:$JAVAPROTOBUF_DIR:../../../src/api/java/gravity.jar -Djava.library.path=../../../src/api/java:$PROTOBUF_INCLUDE_DIR:$ZMQ_LIB_DIR BasicServiceProvider &
JDPPID=$!

# sleep for a second to give the publisher a chance to register its data product
sleep 1

java -cp build:$JAVAPROTOBUF_DIR:../../../src/api/java/gravity.jar -Djava.library.path=../../../src/api/java:$PROTOBUF_INCLUDE_DIR:$ZMQ_LIB_DIR BasicServiceRequestor

kill $SDPID
kill $JDPPID
