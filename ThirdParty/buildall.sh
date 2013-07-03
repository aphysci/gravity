#!/bin/bash

if [ ${#*} -gt 1 ]; then
    echo "Usage: buildall.sh [clean]"
    exit 1
fi

pushd zeromq-3.2.1
# address issue where configure doesn't work right if there are carridge returns.
find . -name "*.in" -exec dos2unix {} \;

./configure || exit 1
make $@ || exit 1
popd

pushd protobuf-2.4.1
./configure || exit 1
make $@ || exit 1
pushd java
popd
popd

rm -rf include/*
find protobuf-2.4.1/src/ -type f -iname "*.h*" -exec cp --parents {} include ";"
mv include/protobuf-2.4.1/src/google include
rm -rf include/protobuf-2.4.1

pushd cppdb-trunk
if [ `uname -o` == "Msys" ]; then
    cmake -G "MSYS Makefiles" || exit 1
else
    cmake . || exit 1
fi
make $@ || exit 1
popd

rm -rf ./lib/*.a ./lib/*.so
find . -path './lib' -prune -o -name *.so* -exec cp -d {} lib \;
find . -path './lib' -prune -o -name *.a -exec cp {} lib \;
find . -path './lib' -prune -o -name *.jar -exec cp {} lib \;

rm -rf ./bin/*
cp protobuf-2.4.1/src/.libs/protoc protobuf-2.4.1/src/.libs/protoc.exe ./bin >& /dev/null
find . -path './bin' -prune -o -name *.dll -exec cp {} bin \;

