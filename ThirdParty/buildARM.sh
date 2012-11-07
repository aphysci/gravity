#!/bin/bash

source /usr/local/angstrom/arm/environment-setup

TP_DIR=`pwd`

pushd zeromq-3.2.1
./configure CXX=arm-angstrom-linux-gnueabi-g++ CC=arm-angstrom-linux-gnueabi-gcc --prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr --host=i386-linux-gnu || exit 1
make clean || exit 1
make || exit 1
popd

pushd protobuf-2.4.1
./configure CXX=arm-angstrom-linux-gnueabi-g++ CC=arm-angstrom-linux-gnueabi-gcc --prefix=/usr/local/angstrom/arm/arm-angstrom-linux-gnueabi/usr --host=i386-linux-gnu --target=arm --with-protoc=${TP_DIR}/bin/protoc
make clean || exit 1
make || exit 1
popd

pushd iniparser
make clean || exit 1
make CROSS=arm-angstrom-linux-gnueabi- || exit 1
popd

rm -rf ./lib/*.a ./lib/*.so
find . -path './lib' -prune -o -name *.so* -exec cp -d {} lib \;
find . -path './lib' -prune -o -name *.a -exec cp {} lib \;
find . -path './lib' -prune -o -name *.jar -exec cp {} lib \;


