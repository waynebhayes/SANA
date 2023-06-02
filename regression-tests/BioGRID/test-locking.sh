#!/bin/bash
CORES=${CORES:=`./scripts/cpus 2>/dev/null || echo 4`}
PARALLEL_EXE=${PARALLEL_EXE:?"PARALLEL_EXE must be set"}
PARALLEL_CMD="$PARALLEL_EXE -s bash $CORES"
echo "PARALLEL_CMD is '$PARALLEL_CMD'" >&2
die() { echo "ERROR: $@" >&2; exit 1
}


PATH="$PATH:`pwd`/scripts"
export PATH

TMPDIR=/tmp/regression-test$$ # gets removed only if everything works
 trap "rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir $TMPDIR

OutputFile="$TMPDIR/regression-lock.$HOST.result"
ErrorMargin="0.04"

> "$OutputFile"

NL='
'

[ -x "$EXE" ] || die "can't find executable '$EXE'"

# ordered by size in number of nodes, then put the list of pairs into file networks.locking
nets="RNorvegicus SPombe CElegans MMusculus SCerevisiae AThaliana DMelanogaster HSapiens"
echo "$nets" | newlines |
    gawk '{net[NR-1]=$NF}END{for(i=0;i<NR;i++)for(j=i+1;j<NR;j++) printf "%s %s\n",net[i],net[j]}' |
    if $CI; then head -$CORES; else cat; fi > $TMPDIR/networks.locking

NUM_FAILS=0
NUM_LOCK=100 # number of nodes to lock
echo -n "Creating random lock files; "
cat $TMPDIR/networks.locking |
	while read g1 g2
	do
	    paste <(cat networks/$g1.el | newlines | sort -u | randomizeLines | head -$NUM_LOCK) <(cat networks/$g2.el | newlines | sort -u | randomizeLines | head -$NUM_LOCK) > $TMPDIR/$g1-$g2.lock
	done
echo "running lock test."

cat $TMPDIR/networks.locking |
    gawk '{printf "echo Running %s-%s; \"'"$EXE"'\" -tolerance 0 -t 1 -s3 1 -g1 %s -g2 %s -lock '$TMPDIR'/%s-%s.lock -o '$TMPDIR'/%s-%s > '$TMPDIR'/%s-%s.progress 2>&1\n",$1,$2,$1,$2,$1,$2,$1,$2,$1,$2}' | tee $TMPDIR/runs.txt | eval $PARALLEL_CMD

PARA_EXIT=$?
echo "'$PARALLEL_CMD' returned $PARA_EXIT; current NUM_FAILS is $NUM_FAILS"

(( NUM_FAILS+=$PARA_EXIT ))

echo "Checking SANA Locking Mechanism" | tee -a $OutputFile
echo 0 > $TMPDIR/failed
cat $TMPDIR/networks.locking |
    while read Network1 Network2; do
	grep -c -x -f "$TMPDIR/$Network1-$Network2.lock" "$TMPDIR/$Network1-$Network2.align"
    done |
    if awk "$NUM_LOCK"'!=$1{++errors}END{exit errors}'; then :
    else
        errors=$?
	echo "LOCKING FAILED in $errors network pairs" >&2
	echo $errors > $TMPDIR/failed
    fi
(( NUM_FAILS+=`cat $TMPDIR/failed` ))

echo encountered a total of $NUM_FAILS failures
if [ "$NUM_FAILS" -ne 0 ]; then
    echo "$NUM_FAILS locks failed; results in directory $TMPDIR">&2
    trap "exit" 0 1 2 3 15
fi

exit $NUM_FAILS
