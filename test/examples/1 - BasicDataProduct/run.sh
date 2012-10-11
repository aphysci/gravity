#!/bin/bash

export PATH=$PATH:../../../ThirdParty/bin:../../../src/components/cpp/bin

ServiceDirectory &
BasicDataProductPublisher &

# sleep for a second to give the publisher a chance to register its data product
sleep 1

BasicDataProductSubscriber