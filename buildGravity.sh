#!/bin/bash

pushd ./ThirdParty >& /dev/null
./buildall.sh || exit 1
popd >& /dev/null

pushd ./src/api >& /dev/null
./buildall.sh || exit 1
popd >& /dev/null

pushd ./src/components/cpp >& /dev/null
./buildall.sh || exit 1
popd >& /dev/null

pushd ./test >& /dev/null
./testall.sh || exit 1
popd >& /dev/null

rm -rf bin lib include
mkdir bin
cp ThirdParty/bin/* bin
cp src/components/cpp/bin/* bin

mkdir lib
cp ThirdParty/lib/* lib
cp src/api/cpp/*.a lib
cp src/api/cpp/*.so lib
cp src/api/java/*.so lib
cp src/api/java/*.jar lib

mkdir include
find src/api/cpp -type f -iname "*.h*" -exec cp --parents {} include ";"
mv include/src/api/cpp/* include
rm -rf include/src
rm -rf include/protobuf
cp -r ThirdParty/include/* include

rm gravity.tgz
tar czf gravity.tgz bin lib include

