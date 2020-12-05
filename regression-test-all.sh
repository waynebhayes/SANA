#!/bin/bash
# Functions
die(){ (echo "$USAGE"; echo "FATAL ERROR: $@")>&2; exit 1; }
warn(){ (echo "WARNING: $@")>&2; }
not(){ if eval "$@"; then return 1; else return 0; fi; }
newlines(){ awk '{for(i=1; i<=NF;i++)print $i}' "$@"; }
parse(){ awk "BEGIN{print $@}" </dev/null; }

# generally useful Variables
NL='
'
TAB='	'

case "$1" in
-use-git-at)
    if [ -f git-at ] && [ `wc -l < git-at` -eq 2 -a `git log -1 --format=%at` -eq `tail -1 git-at` ]; then
	echo -n "Repo unchanged; returning same status code as "
	tail -1 git-at | xargs -I{} date -d @{} +%Y-%m-%d-%H:%M:%S
	exit `head -1 git-at`
    fi
    shift
    ;;
esac

USAGE="USAGE: $0 [ -make ] [ -x SANA_EXE ] [ list of tests to run, defaults to regression-tests/*/*.sh ]"

PATH=`pwd`:`pwd`/scripts:$PATH
export PATH

if [ ! -x NetGO/NetGO.awk ]; then
    echo "you need the submodule NetGO; trying to get it now" >&2
    (git submodule init && git submodule update && cd NetGO && git checkout master && git pull) || die "failed to get NetGO"
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

REAL_CORES=`cpus 2>/dev/null | awk '{print 1*$1}'`
[ "$REAL_CORES" -gt 0 ] || die "can't figure out how many cores this machine has"
CORES=$REAL_CORES
MAKE_CORES=`expr $REAL_CORES - 1`
[ `hostname` = Jenkins ] && MAKE_CORES=2 # only use 2 cores to make on Jenkins
echo "Using $MAKE_CORES cores to make and $CORES cores for regression tests"
export EXE CORES REAL_CORES MAKE_CORES

NUM_FAILS=0
EXECS=`grep '^ifeq (' Makefile | sed -e 's/.*(//' -e 's/).*//' | grep -v MAIN | sort -u`
rm -f parallel
if not make parallel; then
    warn "can't make parallel; using single-threaded shell instead"
    echo "exec bash" > parallel; chmod +x parallel
fi

for EXT in $EXECS ''; do
    if [ "$EXT" = "" ]; then ext='' # no dot
    else ext=.`echo $EXT | tr A-Z a-z` # includes the dot
    fi
    if $MAKE ; then
	[ "$EXT" = "" ] || EXT="$EXT=1"
	make $EXT clean
	if not make -k -j$MAKE_CORES $EXT; then # "-k" means "keep going even if some targets fail"
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
