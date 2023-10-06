#!/bin/sh
USAGE="USAGE: $0 OUTDIR {list of network files, LEDA or edgelist accepted}"
die() { echo "ERROR: $@" >&2; echo "$USAGE" >&2; exit 1
}

newlines() { /bin/awk '{for(i=1; i<=NF; i++) print $i}' "$@"
}
randomizeLines() {
cat "$@" | awk 'BEGIN{srand();srand(int(2^30*rand())+PROCINFO["pid"]+PROCINFO["ppid"])}
    function randint(N){return int(N*rand())}
    {line[NR]=$0}
    END{
        N=NR;
        while(N) {
            k=randint(N)+1;
            print line[k];
            line[k]=line[N--];
        }
    }'
}

OUTDIR="$1"; shift
[ -d "$OUTDIR" ] || die "first argument must be name of output directory, and it must already exist"

TMPDIR=/tmp/multi-pairwise-random$$
 trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir $TMPDIR
GROUP=''

readCols=2 # read the second column 
# NOTE: the above means that, when bipartite, nodes are internally numbered as follows:
# first we read through the first column ONLY, incremeting numNodes as we encounter previously unseen nodes;
# then we do the same for the second column.

for i
do
    b=`basename $i .gw`
    b=`basename $b .el`
    [ -f $TMPDIR/$b.1.nodes ] && die "argument $i: already have a network called $b"
    GROUP="$GROUP-$b"
    case "$i" in
    *.gw) awk '/^.{/&&NF==1{print}' $i | sed 's/[|{}]//g';;
    *.el) awk 'BEGIN{numNodes=0}{for(i=1;i<='$readCols';i++)if(!seen[$i]){seen[$i]=1;name[numNodes++]=$i}}END{for(i=0;i<numNodes;i++)print name[i]}' $i;;
    esac | randomizeLines | tee $TMPDIR/$b.1.nodes | wc -l > $TMPDIR/$b.1.numNodes
done
#GROUP=`echo " $GROUP" | sed 's/^ -//'`
GROUP=group
#SHADOW_NODES=`cd $TMPDIR && grep . *.numNodes | sed -e 's/\.numNodes//' -e 's/:/ /' | sort -k 2n | tail -1 | awk '{print $2}'`
paste $TMPDIR/*.1.nodes | sed -e 's/		/	_	/g' -e 's/		/	_	/g' -e 's/^	/_	/' -e 's/	$/	_/' > $TMPDIR/$GROUP.multiAlign

(
    cd $TMPDIR
    N=0
    for i in *.1.nodes
    do
	b=`basename $i .1.nodes`
	N=`expr $N + 1`
	cut -f$N $GROUP.multiAlign | awk '$1!="_"{printf("%s\tshad_%d\n",$1,NR-1);}' > $b-shadow.align
    done
)
mv $TMPDIR/*[Aa]lign "$OUTDIR"
