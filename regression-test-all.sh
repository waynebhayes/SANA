#!/bin/bash
die() { echo "FATAL ERROR: $@" >&2; exit 1
}
PATH=`pwd`:`pwd`/scripts:$PATH
export PATH

SANA_EXE=./sana
MAKE=false
while [ $# -gt -0 ]; do
    case "$1" in
    -make) MAKE=true; shift;;
    *) [ -x "$1" -o "$MAKE" = true ] || die "unknown argument '$1'; valid arguments are '-make', and an optional sana executable"
	SANA_EXE="$1"; shift;;
    esac
done

export SANA_EXE
if $MAKE ; then
    CORES=`cpus 2>/dev/null || echo 4`
    make clean; make multi -j$CORES; [ -x sana.multi ] || die "could not create executable 'sana.multi'"
    make clean; make -j$CORES; [ -x sana ] || die "could not create 'sana' executable"
    if [ "$SANA_EXE" != ./sana ]; then
	mv -i sana "$SANA_EXE"
	mv -i sana.multi "$SANA_EXE.multi"
    fi
fi

NUM_FAILS=0
for dir in regression-tests/*; do
    echo --- in directory $dir ---
    for r in $dir/*.sh; do
	echo --- running test $r ---
	if bash -x "$r"; then
	    :
	else
	    (( NUM_FAILS+=$? ))
	fi
    done
done
echo Number of failures: $NUM_FAILS
exit $NUM_FAILS
