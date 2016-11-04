#!/bin/bash 


if [ "$#" -ne 1 ]; then
    echo "Usage: ./test.sh <test>"
    exit 1
fi

prog=./cfeeny
tst=$1

echo "***********************************************************************"
echo "                  Testing {$tst}                                       "
echo "***********************************************************************"

$prog output/$tst.ast > testsuite/$tst.out.test
numlines=$(diff testsuite/$tst.out.test testsuite/$tst.out | wc -l)
if [ "$numlines" -gt 4 ]
then
    echo  "Test Fail:{$tst}"
    diff testsuite/$tst.out.test testsuite/$tst.out
else
    echo  "Test Pass:{$tst}"
fi
rm testsuite/$tst.out.test
