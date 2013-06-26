#!/bin/bash

export PATH=$PATH:../../../ThirdParty/bin:../../../src/components/cpp/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../ThirdParty/lib:../../../src/api/cpp

 control_c()
# run if user hits control-c
{
  kill -9 $SDPID
  kill -9 $PUBPID
  exit $?
}

# trap keyboard interrupt (control-c)
trap control_c SIGINT

ServiceDirectory &
SDPID=$!
./BasicDataProductPublisher &
PUBPID=$!

# sleep for a second to give the publisher a chance to register its data product
sleep 1

./BasicDataProductSubscriber
