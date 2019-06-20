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

SCRIPT_DIR=${CMAKE_CURRENT_BINARY_DIR}
pushd $SCRIPT_DIR

testPubSub()
{
    ServiceDirectory &
    SDPID=$!
    ./publisher &
    PUBPID=$!
    ./subscriber &
    SUBPID=$!

    sleep 15
    kill $SDPID $SUBPID
    sleep 4

    ServiceDirectory &
    SDPID=$!

    ./subscriber
    ret=$?

    kill $SDPID $PUBPID

    return $ret
}

testProvReq()
{
    ServiceDirectory &
    SDPID=$!
    ./provider &
    PROVPID=$!
    ./requestor &
    REQPID=$!

    sleep 15
    kill $SDPID $REQPID
    sleep 4

    ServiceDirectory &
    SDPID=$!

    ./requestor
    ret=$?

    kill $SDPID $PROVPID

    return $ret
}

make clean || exit 1
make || exit 1

testPubSub || exit 1

sleep 2

testProvReq || exit 1

popd

exit $ret

