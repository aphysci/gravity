#!/bin/bash

export PATH=$PATH:../../../ThirdParty/bin:../../../src/components/cpp/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../ThirdParty/lib:../../../src/api/cpp

ServiceDirectory &
SDPID=$!
MiscComponent1 &
MC1PID=$!

# sleep for a second to give the publisher a chance to register its data product
sleep 1

MiscComponent2

kill $SDPID
kill $MC1PID
