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

