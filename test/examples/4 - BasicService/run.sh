#!/bin/bash

export PATH=$PATH:../../../src/components/cpp/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../src/api/cpp:../../../src/keyvalue_parser:$ZMQ_LIB_DIR:$PROTOBUF_LIB_DIR

ServiceDirectory &
SDPID=$!
BasicServiceProvider &
BSPPID=$!

# sleep for a second to give the publisher a chance to register its data product
sleep 1

BasicServiceRequestor

kill $SDPID
kill $BSPPID
