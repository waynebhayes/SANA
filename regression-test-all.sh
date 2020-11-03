#!/bin/bash
if [ -f git-at ] && [ `wc -l < git-at` -eq 2 -a `git log -1 --format=%at` -eq `tail -1 git-at` ]; then
    echo -n "Repo unchanged; returning same status code as "
    tail -1 git-at | xargs -I{} date -d @{} +%Y-%m-%d-%H:%M:%S
    exit `head -1 git-at`
fi
USAGE="USAGE: $0 [ -make ] [ -x SANA_EXE ] [ list of tests to run, defaults to regression-tests/*/*.sh ]"
source ~/bin/misc.sh
PATH=`pwd`:`pwd`/scripts:$PATH
export PATH

if [ ! -x NetGO/NetGO.awk ]; then
    echo "you need the submodule NetGO; trying to get it now" >&2
    (git submodule init && git submodule update && cd NetGO && git pull) || die "failed to get NetGO"
    [ -x NetGO/NetGO.awk ] || die "Still can't find NetGO"
fi

EXE=./sana
MAKE=false
while [ $# -gt -0 ]; do
    case "$1" in
    -make) MAKE=true; shift;;
    -x) EXE="$2"; shift 2;;
    -*) die "unknown option '$1";;
    *) break;;
    esac
done
[ -x "$EXE" -o "$MAKE" = true ] || die "Executable '$EXE' must exist or you must specify -make"

CORES=${CORES:=`cpus 2>/dev/null | awk '{c2=int($1/2); if(c2>0)print c2; else print 1}'`}
[ "$CORES" -gt 0 ] || die "can't figure out how many cores this machine has"
MAKE_CORES=$CORES
[ `hostname` = Jenkins ] && MAKE_CORES=2 # only use 2 cores to make on Jenkins
echo "Using $MAKE_CORES cores to make and $CORES cores for regression tests"
export EXE CORES MAKE_CORES

NUM_FAILS=0
EXECS=`grep '^ifeq (' Makefile | sed -e 's/.*(//' -e 's/).*//' | grep -v MAIN | sort -u`
for EXT in $EXECS ''; do
    if [ "$EXT" = "" ]; then ext='' # no dot
    else ext=.`echo $EXT | tr A-Z a-z` # includes the dot
    fi
    if $MAKE ; then
	[ "$EXT" = "" ] || EXT="$EXT=1"
	make $EXT clean
	if not make -k -j$MAKE_CORES $EXT; then # "-k" mean "keep going even if some targets fail"
	    (( NUM_FAILS+=1000 ))
	    warn "make '$EXT' failed"
	fi
	[ $NUM_FAILS -gt 0 ] && warn "Cumulative NUM_FAILS is $NUM_FAILS"
    fi
    [ -x sana$ext ] || warn "sana$ext doesn't exist; did you forget to pass the '-make' option?"
done

STDBUF=''
if which stdbuf >/dev/null; then
    STDBUF='stdbuf -oL -eL'
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
    if eval $STDBUF "$r"; then # force output and error to be line buffered
	:
    else
	NEW_FAILS=$?
	(( NUM_FAILS+=$NEW_FAILS ))
    fi
    echo --- test $r incurred $NEW_FAILS failures, cumulative failures is $NUM_FAILS ---
done
echo Total number of failures: $NUM_FAILS
(echo $NUM_FAILS; git log -1 --format=%at) > git-at
exit $NUM_FAILS
