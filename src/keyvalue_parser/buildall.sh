#!/bin/bash

if [ ${#*} -gt 1 ]; then
    echo "Usage: buildall.sh [clean]"
    exit 1
fi

make $@
