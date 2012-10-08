#!/bin/bash

export PATH=$PATH:../../../ThirdParty/bin:../../../src/components/cpp/bin

ServiceDirectory &
SDPID=$!
# give the SD a second to start up (only seems to be necessary in windows)
sleep 1

make test

kill $SDPID
