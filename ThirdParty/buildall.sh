#!/bin/bash

if [ ${#*} -gt 1 ]; then
    echo "Usage: buildall.sh [clean]"
    exit 1
fi

pushd zeromq-3.2.0
./configure || exit 1
make $@ || exit 1
popd

pushd protobuf-2.4.1
./configure || exit 1
make $@ || exit 1
pushd java
if [ ${#*} -lt 1 ]; then
    mvn -DskipTests package || exit 1
else
    mvn $@ || exit 1
fi
popd
popd

pushd iniparser
make $@ || exit 1
popd

pushd cppdb-trunk
cmake . || exit 1
make $@ || exit 1
popd

