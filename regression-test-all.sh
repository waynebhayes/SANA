#!/bin/bash
die() { echo "FATAL ERROR: $@" >&2; exit 1
}
PATH=`pwd`:`pwd`/scripts:$PATH
export PATH

case "$1" in
-make)
    CORES=`cpus 2>/dev/null || echo 4`
    make clean; make multi -j$CORES; [ -x sana.multi ] || die "could not create executable 'sana.multi'"
    make clean; make -j$CORES; [ -x sana ] || die "could not create 'sana' executable"
    ;;
"") ;;
*) die "unknown argument '$1'" ;;
esac

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
