#!/bin/bash
TESTS="a4m3/codegen_tests/test*"
for file in $TESTS
do
    echo "Input: compile --chk_decl <$file"
    ./compile --chk_decl <$file >myout
    echo $? >>myout
    ./excompile --chk_decl <$file >exout
    echo $? >>exout
    diff myout exout
done
for file in $TESTS
do
    echo "Input: compile --chk_decl --gen_code <$file"
    ./compile --chk_decl --gen_code <$file >myout
    echo $? >>myout
    ./excompile --chk_decl --gen_code <$file >exout
    echo $? >>exout
    diff myout exout
done