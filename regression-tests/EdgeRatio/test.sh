#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing measurement Edge Ratio'
CORES=${CORES:=`./scripts/cpus 2>/dev/null || echo 4`}
PARALLEL_EXE=${PARALLEL_EXE:?"PARALLEL_EXE must be set"}
PARALLEL_CMD="$PARALLEL_EXE -s /bin/bash $CORES"
echo "PARALLEL_CMD is '$PARALLEL_CMD'" >&2

REG_DIR=`pwd`/regression-tests/EdgeRatio
[ -d "$REG_DIR" ] || die "should be run from top-level directory of the SANA repo"
[ -x "$EXE.float" ] || die "can't find executable '$EXE.float'"
(cd "$REG_DIR" && /bin/rm -f *.align *.out *.progress)

nets="150 WMean_con WMean_ocd"
TRIES=10 # sometimes these fail at random; try a few times just in case

while [ $TRIES -gt 0 ]; do
    (( TRIES-- ))
    for network in $nets; do
	file="$REG_DIR/$network"
	# Run SANA to align the graph to itself
	echo "Aligning network $network" >&2
	echo "'$EXE.float' -tolerance 0 -t 5 -fg1 '$file.elw' -fg2 '$file.elw' -er 1 -o '$file' &> '$file.progress'"
    done | eval $PARALLEL_CMD
    PARA_STATUS=$?

    if [ $PARA_STATUS -eq 0 ]; then # runs did not crash, now check results
	nets_failed=''
	for network in $nets; do
	    file="$REG_DIR/$network"
	    # Test if SANA has aligned the graph to itself
	    match_nodes=`awk '$1==$2' $file.align | wc -l`
	    nodes_count=`awk '{ print $1"\n"$2 }' $file.elw | sort -u | wc -l`
	    echo "  EXPECTED matching nodes: $nodes_count GOT: $match_nodes"
	    if (( $match_nodes != $nodes_count )); then
		echo "Failed to align $network to itself; $TRIES tries remaining"
		nets_failed="$nets_failed $network"
	    fi
	done
	[ "$nets_failed" = "" ] && break
	nets="$nets_failed"
    fi
done
NUM_FAILS=`echo $nets_failed | wc -w`

echo "Done testing measurement Edge Ratio; $NUM_FAILS failures"
exit $NUM_FAILS
