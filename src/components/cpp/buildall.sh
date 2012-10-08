#!/bin/bash

if [ ${#*} -gt 1 ]; then
    echo "Usage: buildall.sh [clean]"
    exit 1
fi

rm -rf ./bin/*
for dirnm in `ls .`
do
	if [ -d ${dirnm} ] && [ -f ./${dirnm}/Makefile ]
	then
		rm -f ./${dirnm}/${dirnm} ./bin/${dirnm}
		pushd ${dirnm} >& /dev/null
		pwd
#		make clean || exit 1
		make $@ || exit 1
		if [ -f ${dirnm} ]; then
			cp ./${dirnm} ../bin/
		fi
		popd >& /dev/null
	fi
done

