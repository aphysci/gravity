#!/bin/bash

for script in `find . -name test.sh` 
do
    $script || exit 1
done

echo
echo All tests completed Successfully!!!
echo
