#!/bin/bash
# Functions
die(){ (echo "$USAGE"; echo "FATAL ERROR: $@")>&2; exit 1; }
warn(){ (echo "WARNING: $@")>&2; }
not(){ if eval "$@"; then return 1; else return 0; fi; }
newlines(){ awk '{for(i=1; i<=NF;i++)print $i}' "$@"; }
parse(){ gawk "BEGIN{print $@}" </dev/null; }

[ "$CI" = true ] || export CI=false

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

CWD=`pwd`
PATH="$CWD:$CWD/scripts:$CWD/NetGO:$PATH"
export PATH
export SANA_TOLERANCE=0 # SANA now uses a tolerance by default, but tons of regression tests require time or iterations

if [ ! -x NetGO/NetGO.awk ]; then
    echo "you need the submodule NetGO; trying to get it now" >&2
    (git submodule init && git submodule update && cd NetGO && git checkout master && (git pull||exit 0)) || die "failed to get NetGO"
    [ -x NetGO/NetGO.awk ] || die "Still can't find NetGO"
fi

export SANA_VER=2.0
export SANA_EXE="${SANA_EXE:=./sana$SANA_VER}"
export SANA_DIR="${SANA_DIR:=`/bin/pwd`}"
MAKE=false
while [ $# -gt -0 ]; do
    case "$1" in
    -make) MAKE=true; shift;;
    -x) EXE="$2"; shift 2;;
    -*) die "unknown option '$1";;
    *) break;;
    esac
done

REAL_CORES=`(cpus 2>/dev/null || echo 1) | awk '{print 1*$1}'`
MAX_THREADS=`(cpus -t 2>/dev/null || echo 1) | awk '{print 1*$1}'`
echo "Machine has $REAL_CORES real cores and $MAX_THREADS total threads"
[ "$REAL_CORES" -gt 0 ] || die "can't figure out how many cores this machine has"
if "$CI"; then
    CORES=$REAL_CORES
    MAKE_CORES=$REAL_CORES
else
    if [ $MAX_THREADS -gt $REAL_CORES ]; then
	CORES=`expr $MAX_THREADS \* 3 / 4`
	MAKE_CORES=$CORES
    else
	CORES=`expr $REAL_CORES - 1`
	MAKE_CORES=`expr $REAL_CORES - 1`
    fi
fi
[ `hostname` = Jenkins ] && MAKE_CORES=2 # only use 2 cores to make on Jenkins
echo "Using $MAKE_CORES threads to make and $CORES threads for regression tests"

NUM_FAILS=0
export EXECS=`sed '/MAIN=error/q' Makefile | grep '^ifeq (' | sed -e 's/.*(//' -e 's/).*//' | egrep -v "MAIN|[<>]"`
[ `echo $EXECS | newlines | wc -l` -eq `echo $EXECS | newlines | sort -u | wc -l` ] || die "Internal error: list of EXECS <$EXECS> contains duplicates"

export PARALLEL_EXE=/tmp/parallel.$$
 trap "exit" 0 1 2 3 15
if (echo echo hello | ./parallel 1 | grep '^hello$') >/dev/null 2>&1; then
    :
else
    rm -f parallel $PARALLEL_EXE
    if make parallel; then
	:
    else
	warn "can't make parallel; using single-threaded shell instead"
	echo 'if [ "$1" = -s ]; then shift 2; fi; shift; exec bash' > parallel
    fi
fi
cp -p parallel $PARALLEL_EXE; chmod +x parallel $PARALLEL_EXE

WORKING_EXECS='' #TAB separated full names of executables
for EXT in '' $EXECS; do
    if [ "$EXT" = "" ]; then ext='' # no dot
    else
	ext=.`echo $EXT | tr A-Z a-z` # includes the dot
	if [ "$EXT" = MPI ] && not which mpicxx > /dev/null; then
	    warn "skipping $EXT ($SANA_EXE$ext) because mpicxx could not be found"
	    continue
	fi
    fi
    if $MAKE ; then
	echo -n "Attempting to make $SANA_EXE$ext... "
	[ "$EXT" = "" ] || EXT="$EXT=1"
	make $EXT clean > $SANA_EXE$ext.make.log 2>&1
	if make -k -j$MAKE_CORES $EXT >> $SANA_EXE$ext.make.log 2>&1; then # "-k" means "keep going even if some targets fail"
	    echo "SUCCESS!"
	else
	    warn "make '$EXT' failed; see '$SANA_EXE$ext.make.log' for details"
	    if echo "$ext" "$EXT" | egrep -i 'static|mpi' >/dev/null; then warn "ignoring failure of make '$EXT'";
	    else (( ++NUM_FAILS ));
	    fi
	    
	fi
	[ $NUM_FAILS -eq 0 ] || warn "cumulative number of compile failures is $NUM_FAILS"
    fi
    #[ -x "$EXE" ] || die "Executable '$EXE' must exist or you must specify -make"
    # skip mpi, multi and float since they will be tested separately below
    [ "$ext" = .multi -o "$ext" = .float -o "$ext" = .mpi ] && continue
    if ./$SANA_EXE$ext -tolerance 0 -itm 1 -s3 1 -g1 yeast -g2 human -tinitial 1 -tdecay 1 >/dev/null 2>&1; then
	WORKING_EXECS="${WORKING_EXECS}$SANA_EXE$ext$TAB"
    else
	warn "executable $SANA_EXE$ext failed a trivial test"
	#rm -f $SANA_EXE$ext
    fi
done
(( NUM_FAILS *= 1000 ))
WORKING_EXECS=`echo "$WORKING_EXECS" | sed 's/	$//'` # delete trailing TAB

# Now sort them so sana.sparse is first and sana.cores is last, to minimize memory usage
WORKING_EXECS=`echo "$WORKING_EXECS" | newlines |
    awk 'BEGIN{n=0; done["'$SANA_EXE'.sparse"]=1; list[++n]="'$SANA_EXE'.sparse"}
	!done[$0]{done[$0]=1; if($0!="'$SANA_EXE'.cores")list[++n]=$0;}
	END{if(done["'$SANA_EXE'.cores"]) list[++n]="'$SANA_EXE'.cores";
	    for(i=1;i<=n;i++) print list[i]}'`

export SANA_EXE SANA_DIR CORES REAL_CORES MAKE_CORES EXECS WORKING_EXECS

echo WORKING EXECUTABLES: ${WORKING_EXECS:?"${NL}FATAL ERROR: cannot continue since there are no working pairwise executables!$NL(with the possible exception of float)"}

STDBUF=''
if which stdbuf >/dev/null; then
    STDBUF='stdbuf -oL -eL'
fi

if [ $# -eq 0 ]; then
    set regression-tests/*/*.sh
fi

for r
do
    REG_DIR=`dirname $r | head -1`
    NEW_FAILS=0
    export REG_DIR
    echo --- running test "'$r'" ---
    if eval time $STDBUF $r; then # force output and error to be line buffered
	:
    else
	NEW_FAILS=$?
	(( NUM_FAILS+=$NEW_FAILS ))
    fi
    echo --- test "'$r'" incurred $NEW_FAILS failures, cumulative failures is $NUM_FAILS ---
done
echo Total number of failures: $NUM_FAILS
(echo $NUM_FAILS; git log -1 --format=%at) > git-at
exit $NUM_FAILS
