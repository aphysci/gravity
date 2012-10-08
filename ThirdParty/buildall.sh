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
if [ `uname -o` == "Msys" ]; then
    cmake -G "MSYS Makefiles" || exit 1
else
    cmake . || exit 1
fi
make $@ || exit 1
popd

rm -rf ./lib/*
find . -path './lib' -prune -o -name *.so -exec cp {} lib \;
find . -path './lib' -prune -o -name *.a -exec cp {} lib \;

rm -rf ./bin/*
find . -path './bin' -prune -o -regex '.*/protoc[\.ex]*' -exec cp {} bin \;


