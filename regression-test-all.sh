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
CORES=${CORES:=`cpus 2>/dev/null || echo 4`}
if $MAKE ; then
    for ext in multi float ''; do
	if [ `hostname` = Jenkins ]; then
	    make clean; make $ext -j2
	else
	    make clean; make $ext -j$CORES
	fi
	# We only want a "." separator if the extension is non-null
	if [ "$ext" = "" ]; then dot=""; else dot="."; fi
	if [ -x sana$dot$ext ]; then
	    mv -f "sana$dot$ext" "$SANA_EXE$dot$ext"
	else
	    warn "could not create executable 'sana$dot$ext'"
	fi
    done
fi

NUM_FAILS=0
for REG_DIR in regression-tests/*; do
    export REG_DIR
    echo --- in directory $REG_DIR ---
    for r in $REG_DIR/*.sh; do
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
