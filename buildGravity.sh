#!/bin/bash
DO_CLEAN=1
DO_TEST=1

echo "Sourcing the Gravity environment variables ..."
source ./gravity_env.config

pushd ./ThirdParty >& /dev/null
if [ $DO_CLEAN == 1 ]; then ./buildall.sh clean || exit 1 ; fi
./buildall.sh || exit 1
popd >& /dev/null

pushd ./src/keyvalue_parser >& /dev/null
if [ $DO_CLEAN == 1 ]; then ./buildall.sh clean || exit 1  ; fi
./buildall.sh || exit 1

popd >& /dev/null
pushd ./src/api >& /dev/null
if [ $DO_CLEAN == 1 ]; then ./buildall.sh clean || exit 1  ; fi
./buildall.sh || exit 1
popd >& /dev/null

pushd ./src/components/cpp >& /dev/null
if [ $DO_CLEAN == 1 ]; then ./buildall.sh clean || exit 1  ; fi
./buildall.sh || exit 1
popd >& /dev/null

if [ $DO_TEST == 1 ]; then
  pushd ./test >& /dev/null
  ./testall.sh || exit 1
  popd >& /dev/null
fi

rm -rf bin lib include
mkdir bin
cp ThirdParty/bin/* bin
cp src/components/cpp/bin/* bin

mkdir lib
cp -d ThirdParty/lib/* lib
cp src/keyvalue_parser/*.a lib
cp src/keyvalue_parser/*.so lib
cp src/api/cpp/*.a lib
cp src/api/cpp/*.so lib
cp src/api/java/*.so lib
cp src/api/java/*.jar lib

mkdir include
cp -r src/api/cpp/*.h include
mkdir include/protobuf
cp src/api/cpp/protobuf/GravityDataProductPB.pb.h include/protobuf
cp src/api/cpp/protobuf/GravityMetricsDataPB.pb.h include/protobuf
cp -r ThirdParty/include/* include
mkdir include/MATLAB
cp src/api/MATLAB/*.m include/MATLAB
mkdir lib/MATLAB
cp src/api/MATLAB/*.jar lib/MATLAB

echo building Gravity tarball...
rm gravity.tgz >& /dev/null
tar czf gravity.tgz bin lib include

