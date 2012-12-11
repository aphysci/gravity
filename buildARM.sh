#!/bin/bash -x

echo starting Gravity ARM build...
# build native to get the protoc executable
pushd ThirdParty
pushd protobuf-2.4.1
./configure CC=gcc CXX=g++
make clean || exit 1
make || exit 1
popd

mkdir ./bin
rm -rf ./bin/protoc
cp protobuf-2.4.1/src/.libs/protoc ./bin 
popd

# build the rest with the arm settings
source /usr/local/angstrom/arm/environment-setup

pushd ./ThirdParty 
./buildARM.sh || exit 1
popd 

pushd ./src/api/cpp 
make clean || exit 1
make CROSS=arm-angstrom-linux-gnueabi- || exit 1
popd 

pushd ./src/components/cpp/ServiceDirectory 
make clean || exit 1
make CROSS=arm-angstrom-linux-gnueabi- || exit 1
popd 

pushd ./src/components/cpp/LogRecorder 
make clean || exit 1
make CROSS=arm-angstrom-linux-gnueabi- || exit 1
popd 

rm -rf bin lib include
mkdir bin
cp src/components/cpp/ServiceDirectory/ServiceDirectory bin
cp src/components/cpp/LogRecorder/LogRecorder bin

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

echo building Gravity ARM tarball...
rm gravityARM.tgz 
tar czf gravityARM.tgz bin lib include

