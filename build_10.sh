#!/bin/bash 
echo "***********************************************************************"
echo "                                                                       "
echo "                  Building Cfeeny                                      "
echo "                                                                       "
echo "***********************************************************************"
clang -std=c99 src/cfeeny.c src/utils.c src/ast.c
src/bytecode.c src/compiler.c src/vm.c -o cfeeny 
