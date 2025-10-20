#!/bin/bash

OK_TESTS=$(ls parser_tests/test[0-9]*)
ERR_TESTS=$(ls parser_tests/err[0-9]*)
for file in $OK_TESTS
do
    echo "INPUT: $file"
    compile < $file
    excompile < $file
done

for file in $ERR_TESTS
do
    echo "INPUT: $file"
    compile < $file
    excompile < $file
done
