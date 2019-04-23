#!/bin/sh
RETURN=0
for dir in regression-tests/*; do
    echo --- in directory $dir ---
    for r in $dir/*.sh; do
	echo --- running test $r ---
	if nice -19 "$r"; then
	    :
	else
	    RETURN=1
	fi
    done
done
exit $RETURN
