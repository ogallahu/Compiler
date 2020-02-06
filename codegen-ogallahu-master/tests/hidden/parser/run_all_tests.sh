#!/bin/sh

export PATH=$PATH:../../../

EXIT_STATUS=0

for testfile in good*.bminor
do
	if bminor -parse "$testfile" > "$testfile.out"
	then
		echo "$testfile success (as expected)"
	else
		echo "$testfile failure (INCORRECT)"
		EXIT_STATUS=1
	fi
done

for testfile in bad*.bminor
do
	if bminor -parse "$testfile" > "$testfile.out"
	then
		echo "$testfile success (INCORRECT)"
		EXIT_STATUS=1
	else
		echo "$testfile failure (as expected)"
	fi
done

exit $EXIT_STATUS