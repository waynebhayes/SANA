#!/bin/sh
USAGE="USAGE: $0 OUTDIR {list of network files, LEDA or edgelist accepted}"
die() { echo "ERROR: $@" >&2; echo "$USAGE" >&2; exit 1
}

OUTDIR="$1"; shift
[ -d "$OUTDIR" ] || die "first argument must be name of output directory"

TMPDIR=/tmp/multi-pairwise-random$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir $TMPDIR
GROUP=''
for i
do
    b=`basename $i .gw`
    b=`basename $b .el`
    GROUP="$GROUP-$b"
    case "$i" in
    *.gw) awk '/^.{/&&NF==1{print}' $i | sed 's/[|{}]//g';;
    *.el) awk '{for(i=1;i<=NF;i++)n[$i]=1}END{for(i in n)print i}' $i;;
    esac | randomizeLines | tee $TMPDIR/$b.nodes | wc -l > $TMPDIR/$b.numNodes
done
GROUP=`echo " $GROUP" | sed 's/^ -//'`
SHADOW_NODES=`cd $TMPDIR && grep . *.numNodes | sed -e 's/\.numNodes//' -e 's/:/ /' | sort -k 2n | tail -1 | awk '{print $2}'`
paste $TMPDIR/*.nodes | sed -e 's/		/	_	/g' -e 's/		/	_	/g' -e 's/^	/_	/' -e 's/	$/	_/' > $TMPDIR/$GROUP.align

(
    cd $TMPDIR
    N=0
    for i in *.nodes
    do
	b=`basename $i .nodes`
	N=`expr $N + 1`
	cut -f$N $GROUP.align | awk '{printf "shadow%d\t%s\n",NR-1,$1}' > shadow-$b.align
    done
)
mv $TMPDIR/*align "$OUTDIR"
