#!/bin/sh
RETURN=0
for dir in regression-tests/*; do
    for r in $dir/*.sh; do
	if nice -19 "$r"; then
	    :
	else
	    RETURN=1
	fi
    done
done
exit $RETURN
