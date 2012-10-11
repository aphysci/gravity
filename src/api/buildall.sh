#!/bin/bash

if [ ${#*} -gt 1 ]; then
    echo "Usage: buildall.sh [clean]"
    exit 1
fi

pushd cpp
make $@ || exit 1
popd

pushd java
make $@ || exit 1
popd


