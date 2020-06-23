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
NUM_FAILS=0
CORES=${CORES:=`cpus 2>/dev/null || echo 4`}
if $MAKE ; then
    for EXT in `grep '^ifeq (' Makefile | sed -e 's/.*(//' -e 's/).*//' | grep -v MAIN | sort -u` ''; do
	if [ "$EXT" = "" ]; then ext=''
	else ext=`echo $EXT | tr A-Z a-z`
	fi
	if [ `hostname` = Jenkins ]; then # make -k means "keep going even if some targets fail"
	    make clean; make -k -j2 "$EXT=1" || warn "make '$EXT=1' failed" && (( NUM_FAILS+=1000 ))
	else
	    make clean; make -k -j$CORES "$EXT=1" || warn "make '$EXT=1' failed" && (( NUM_FAILS+=1000 ))
	fi
	[ $NUM_FAILS -gt 0 ] && warn "Cumulative NUM_FAILS is $NUM_FAILS"
    done
fi

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
