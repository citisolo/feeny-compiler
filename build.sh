#!/bin/bash 
echo "***********************************************************************"
echo "                                                                       "
echo "                  Building Cfeeny                                      "
echo "                                                                       "
echo "***********************************************************************"
clang -std=c99  src/utils.c src/ast.c src/bytecode.c src/compiler.c src/vm.c src/cfeeny.c -g -o cfeeny 
