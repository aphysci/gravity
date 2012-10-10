#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR" &> /dev/null

export PATH=$PATH:../../../ThirdParty/bin:../../../src/components/cpp/bin

ServiceDirectory &
SDPID=$!
# give the SD a second to start up (only seems to be necessary in windows)
sleep 1

BasicDataProductPublisher

kill $SDPID

popd &> /dev/null

exit $ret
