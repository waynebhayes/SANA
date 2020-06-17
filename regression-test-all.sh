#!/bin/bash
USAGE="USAGE: $0 [ -make ] [ -x SANA_EXE ] [ list of tests to run, defaults to regression-tests/*/*.sh ]"
NL='
'
die() { echo "$USAGE$NL FATAL ERROR: $@" >&2; exit 1
}
warn() { echo "WARNING: $@" >&2;
}
PATH=`pwd`:`pwd`/scripts:$PATH
export PATH
export HOST=`hostname|sed 's/\..*//'`

SANA_EXE=./sana
MAKE=false
while true; do
    case "$1" in
    -make) MAKE=true; shift;;
    -x) [ -x "$2" -o "$MAKE" = true ] || die "unknown argument '$2'"
	SANA_EXE="$2"; shift 2;;
    -*) die "unknown option '$1";;
    *) break;;
    esac
done

export SANA_EXE
CORES=${CORES:=`cpus 2>/dev/null || echo 4`}
if $MAKE ; then
    for ext in multi ''; do
	if [ `hostname` = Jenkins ]; then
	    make clean; make $ext -j2 || die "make '$ext' failed"
	else
	    make clean; make $ext -j$CORES || die "make '$ext' failed"
	fi
	# We only want a "." separator if the extension is non-null
	if [ "$ext" = "" ]; then dot=""; else dot="."; fi
	if [ -x sana$dot$ext ]; then
	    [ "sana$dot$ext" != "$SANA_EXE$dot$ext" ] && mv -f "sana$dot$ext" "$SANA_EXE$dot$ext"
	else
	    die "could not create executable 'sana$dot$ext'"
	fi
    done
fi

NUM_FAILS=0
if [ $# -eq 0 ]; then
    set regression-tests/*/*.sh
fi
for r
do
    REG_DIR=`dirname "$r"`
    NEW_FAILS=0
    export REG_DIR
    echo --- running test $r ---
    if "$r"; then
	:
    else
	NEW_FAILS=$?
	(( NUM_FAILS+=$NEW_FAILS ))
    fi
    echo --- test $r incurred $NEW_FAILS failures, cumulative failures is $NUM_FAILS ---
done
echo Number of failures: $NUM_FAILS
exit $NUM_FAILS
