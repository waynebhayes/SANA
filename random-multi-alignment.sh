#!/bin/sh
USAGE="USAGE: $0 [-nodes-have-types] OUTDIR {list of network files, LEDA or edgelist accepted}"
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

TYPES=false
case "$1" in
-nodes-have-types) TYPES=true; shift ;;
esac

OUTDIR="$1"; shift
[ -d "$OUTDIR" ] || die "first argument must be name of output directory"

TMPDIR=/tmp/multi-pairwise-random$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir $TMPDIR
GROUP=''
outCols=1
if $TYPES; then otherCOL=1; outCols=2; fi
for i
do
    b=`basename $i .gw`
    b=`basename $b .el`
    GROUP="$GROUP-$b"
    case "$i" in
    *.gw) if $TYPES; then die "network '$i': can't accept .gw files with -nodes-have-types";fi
	awk '/^.{/&&NF==1{print}' $i | sed 's/[|{}]//g';;
    *.el) awk '{for(i=1;i<=3-'$outCols';i++)n[$i]=1}END{for(i in n)print i}' $i;;
    esac | randomizeLines | tee $TMPDIR/$b.1.nodes | wc -l > $TMPDIR/$b.1.numNodes
    if $TYPES; then awk '{n[$2]=1}END{for(i in n)print i}' $i | randomizeLines | tee $TMPDIR/$b.2.nodes | wc -l > $TMPDIR/$b.2.numNodes; fi
done
GROUP=`echo " $GROUP" | sed 's/^ -//'`
#SHADOW_NODES=`cd $TMPDIR && grep . *.numNodes | sed -e 's/\.numNodes//' -e 's/:/ /' | sort -k 2n | tail -1 | awk '{print $2}'`
paste $TMPDIR/*.1.nodes | sed -e 's/		/	_	/g' -e 's/		/	_	/g' -e 's/^	/_	/' -e 's/	$/	_/' > $TMPDIR/$GROUP.align
paste $TMPDIR/*.2.nodes 2>/dev/null | sed -e 's/		/	_	/g' -e 's/		/	_	/g' -e 's/^	/_	/' -e 's/	$/	_/' >> $TMPDIR/$GROUP.align

(
    cd $TMPDIR
    N=0
    for i in *.1.nodes
    do
	b=`basename $i .1.nodes`
	N=`expr $N + 1`
	cut -f$N $GROUP.align | awk '{printf "shadow%d\t%s\n",NR-1,$1}' > shadow-$b.align
    done
)
mv $TMPDIR/*align "$OUTDIR"
