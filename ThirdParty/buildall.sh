#!/bin/bash

pushd zeromq-3.2.0
./configure
make
popd

pushd protobuf-2.4.1
./configure
make
pushd java
mvn install
popd
popd

pushd iniparser
make
popd

pushd cppdb-trunk
cmake .
make
popd

