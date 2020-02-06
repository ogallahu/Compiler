#!/bin/sh

export PATH=$PATH:../../../

for testfile in good*.bminor
do
    if bminor -print $testfile > $testfile.out
    then
        echo "$testfile success (as expected)"
    else
        echo "$testfile failure (INCORRECT)"
    fi
done

for testfile in bad*.bminor
do
    if bminor -print $testfile > $testfile.out
    then
        echo "$testfile success (INCORRECT)"
    else
        echo "$testfile failure (as expected)"
    fi
done
