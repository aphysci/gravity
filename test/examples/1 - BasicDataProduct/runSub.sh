#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR"

export PATH=$PATH:../../../ThirdParty/bin:../../../src/components/cpp/bin

BasicDataProductSubscriber 

popd

