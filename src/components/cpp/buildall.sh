#!/bin/bash
#** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#**
#** Gravity is free software; you can redistribute it and/or modify
#** it under the terms of the GNU Lesser General Public License as published by
#** the Free Software Foundation; either version 3 of the License, or
#** (at your option) any later version.
#**
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#** GNU Lesser General Public License for more details.
#**
#** You should have received a copy of the GNU Lesser General Public
#** License along with this program;
#** If not, see <http://www.gnu.org/licenses/>.
#**


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

