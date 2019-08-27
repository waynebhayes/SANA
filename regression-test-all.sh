#!/bin/sh
PATH=`pwd`/scripts:$PATH
export PATH
EXIT_CODE=0
for dir in regression-tests/*; do
    echo --- in directory $dir ---
    for r in $dir/*.sh; do
	echo --- running test $r ---
	if nice -19 "$r"; then
	    :
	else
	    EXIT_CODE=1
	fi
    done
done
exit $EXIT_CODE
