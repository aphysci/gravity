#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

export PATH=$PATH:../../../../src/components/cpp/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../../src/api/cpp:../../../../src/keyvalue_parser:$ZMQ_LIB_DIR:$PROTOBUF_LIB_DIR

ServiceDirectory &
SDPID=$!

make clean
make test
ret=$?

kill $SDPID

popd

exit $ret
