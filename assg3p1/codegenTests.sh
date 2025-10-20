#!/bin/bash

OK_TESTS=$(ls codegen_tests/test*)
ERR_TESTS=$(ls codegen_tests/err*)
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
    compile --chk_decl --gen_code < $file >myout
    excompile --chk_decl --gen_code < $file >exout
    diff myout exout
done
