#!/bin/bash
PATH=`pwd`/scripts:$PATH
export PATH
NUM_FAILS=0
for dir in regression-tests/*; do
    echo --- in directory $dir ---
    for r in $dir/*.sh; do
	echo --- running test $r ---
	if nice -19 "$r"; then
	    :
	else
	    (( NUM_FAILS+=$? ))
	fi
    done
done
echo Number of failures: $NUM_FAILS
exit $NUM_FAILS
