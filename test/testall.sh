#!/bin/bash

for script in `find . -name test.sh` 
do
    echo Running $script
    echo
    $script || exit 1

    echo
    echo Success!
done

echo
echo All tests completed Successfully
echo
