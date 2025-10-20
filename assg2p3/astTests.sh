#!/bin/bash

OK_TESTS=$(ls astgen_tests/test[0-9]*)
ERR_TESTS=$(ls astgen_tests/err[0-9]*)
for file in $OK_TESTS
do
    echo "INPUT: $file"
    compile --chk_decl < $file >myout
    excompile --chk_decl < $file >exout
    diff myout exout
done

for file in $OK_TESTS
do
    echo "INPUT: $file"
    compile --chk_decl --print_ast < $file >myout
    excompile --chk_decl --print_ast < $file >exout
    diff myout exout
done
