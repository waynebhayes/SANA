#!/bin/bash
die() { echo "FATAL ERROR: $@" >&2; exit 1
}
warn() { echo "WARNING: $@" >&2;
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
    dot='' # This is a cheap hack to ensure the "." only appears after the $ext is equal to something
    for ext in '' multi float; do
	make clean; make $ext -j$CORES
	if [ -x sana$dot$ext ]; then
	    mv -f sana$dot$ext "$SANA_EXE$dot$ext"
	else
	    warn "could not create executable 'sana$dot$ext'"
	fi
	dot=. # See cheap hack comment above
    done
fi

NUM_FAILS=0
for dir in regression-tests/*; do
    echo --- in directory $dir ---
    for r in $dir/*.sh; do
	echo --- running test $r ---
	if "$r"; then
	    :
	else
	    (( NUM_FAILS+=$? ))
	fi
    done
done
echo Number of failures: $NUM_FAILS
exit $NUM_FAILS
