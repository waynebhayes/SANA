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

if [ ! -f NetGO/awkcel ]; then
    echo "you need the submodule NetGO; trying to get it now" >&2
    (git submodule init NetGO && git submodule update NetGO && cd NetGO && git checkout master && git pull) || die "failed to get NetGO"
    [ -f NetGO/awkcel ] || die "Still can't find NetGO"
fi

SANA_EXE=./sana
DO_MAKE=false
while true; do
    case "$1" in
    -make) DO_MAKE=true; shift;;
    -x) [ -x "$2" -o "$DO_MAKE" = true ] || die "unknown argument '$2'"
	SANA_EXE="$2"; shift 2;;
    -*) die "unknown option '$1";;
    *) break;;
    esac
done

export SANA_EXE
CORES=${CORES:=`cpus -t 2>/dev/null || echo 4`}
echo "Found $CORES threaded cores/cpus"
CORES=`expr $CORES - 1`
echo "Using $CORES cores for parallel"
export MAKE="make -j$CORES"
export CORES
if $DO_MAKE ; then
    for ext in sparse multi ''; do
	if [ `hostname` = Jenkins ]; then
	    make clean; $MAKE $ext -j2 || die "make '$ext' failed"
	else
	    make clean; $MAKE $ext -j$CORES || die "make '$ext' failed"
	fi
	# We only want a "." separator if the extension is non-null
	if [ "$ext" = "" ]; then dot=""; else dot="."; fi
	if [ -x sana$dot$ext ]; then
	    [ "./sana$dot$ext" != "$SANA_EXE$dot$ext" ] && mv -f "sana$dot$ext" "$SANA_EXE$dot$ext"
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
    if time "$r"; then
	:
    else
	NEW_FAILS=$?
	(( NUM_FAILS+=$NEW_FAILS ))
    fi
    echo --- test $r incurred $NEW_FAILS failures, cumulative failures is $NUM_FAILS ---
done
echo Number of failures: $NUM_FAILS
exit $NUM_FAILS
