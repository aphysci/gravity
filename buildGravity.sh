#!/bin/bash

pushd ./ThirdParty >& /dev/null
./buildall.sh || exit 1
popd >& /dev/null

pushd ./src/api >& /dev/null
./buildall.sh clean || exit 1
./buildall.sh || exit 1
popd >& /dev/null

pushd ./src/components/cpp >& /dev/null
./buildall.sh clean || exit 1
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
cp -r src/api/cpp/*.h include
mkdir include/protobuf
cp src/api/cpp/protobuf/GravityDataProductPB.pb.h include/protobuf
cp -r ThirdParty/include/* include
mkdir include/MATLAB
cp src/api/MATLAB/*.m include/MATLAB
mkdir lib/MATLAB
cp src/api/MATLAB/*.jar lib/MATLAB

echo building Gravity tarball...
rm gravity.tgz
tar czf gravity.tgz bin lib include

