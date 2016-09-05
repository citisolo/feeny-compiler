#!/bin/bash 

if [ "$#" -ne 1 ]; then
    echo "Usage: ./create_test_files.sh <filestub>"
    exit 1
fi

testfile=$1

echo "Generating $testfile.ast"
util/parser -i tests/$testfile.feeny -oast output/$testfile.ast
echo "Generating $testfile.ast.out"
util/ast_printer output/$testfile.ast > testsuite/$testfile.ast.out
echo "Generating $testfile.out"
util/compiler -i tests/$testfile.feeny -o $testfile.bc
util/bytecode_printer $testfile.bc > testsuite/$testfile.out
rm $testfile.bc

