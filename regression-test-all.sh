#!/bin/bash
USAGE="USAGE: $0 [ -make ] [ -x SANA_EXE ] [ list of tests to run, defaults to regression-tests/*/*.sh ]"
NL='
'
die() { echo "$USAGE${NL}FATAL ERROR: $@" >&2; exit 1
}
warn() { echo "WARNING: $@" >&2;
}
not() { if eval "$@"; then return 1; else return 0; fi
}
PATH=`pwd`:`pwd`/scripts:$PATH
export PATH
export HOST=`hostname|sed 's/\..*//'`

if [ ! -x NetGO/NetGO.awk ]; then
    echo "you need the submodule NetGO; trying to get it now" >&2
    (git submodule init && git submodule update && cd NetGO && git pull) || die "failed to get NetGO"
    [ -x NetGO/NetGO.awk ] || die "Still can't find NetGO"
fi

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
CORES=$((${CORES:=`cpus 2>/dev/null || echo 4`}-1))
MAKE_CORES=$CORES
[ `hostname` = Jenkins ] && MAKE_CORES=2 # only use 2 cores to make on Jenkins

EXECS=`grep '^ifeq (' Makefile | sed -e 's/.*(//' -e 's/).*//' | grep -v MAIN | sort -u`
for EXT in $EXECS ''; do
    if [ "$EXT" = "" ]; then ext='' # no dot
    else ext=.`echo $EXT | tr A-Z a-z` # includes the dot
    fi
    if $MAKE ; then
	[ "$EXT" = "" ] || EXT="$EXT=1"
	make clean
	if not make -k -j$MAKE_CORES $EXT; then
	    (( NUM_FAILS+=1000 ))
	    warn "make '$EXT' failed"
	fi
	[ $NUM_FAILS -gt 0 ] && warn "Cumulative NUM_FAILS is $NUM_FAILS"
    fi
    [ -x sana$ext ] || warn "sana$ext doesn't exist; did you forget to pass the '-make' option?"
done

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
