#!/bin/sh

PARALLEL="parallel -s sh 8"

TMPDIR=/tmp/overseer.$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir $TMPDIR

USAGE="USAGE: $0 sana.exe iterations outdir {list of input networks}"
die() { echo "$USAGE" >&2; echo "$@" >&2; exit 1
}
warn() { echo "WARNING: $@" >&2
}
parallel_delay() {
    cat > $TMPDIR/pd
    head -1 $TMPDIR/pd | sh &
    until [ -f "$1" ]; do sleep 1; done
    tail -n +2 $TMPDIR/pd | $PARALLEL
}

SANA="$1"
ITER_EXPR="$2"
OUTDIR="$3"
NAME=`basename "$OUTDIR"`
export SANA ITER_EXPR NAME
shift 3

[ -x "$SANA" ] || die "first argument '$SANA' must be an executable file"
ITER=`parse "$ITER_EXPR"` || die "'$ITER_EXPR': cannot figure out iteration count"
if [ -d "$OUTDIR" ]; then
    warn "outdir '$OUTDIR' already exists; continuing"
    MAX_NODES=`lss $OUTDIR/dir-init/*.align | awk 'NR==1{print $NF}' | xargs cat | wc -l` || die "Couldn't figure out value of MAX_NODES from '$OUTDIR/dir-init/*.align'"
else
    mkdir -p "$OUTDIR"/dir-init || die "Cannot make outdir '$OUTDIR'"

    # NOTE: REMAINDER OF THE COMMAND LINE IS ALL THE INPUT NETWORKS

    # Create initial random alignment, which also tells us the number of nodes.
    ./random-multi-alignment.sh "$OUTDIR"/dir-init "$@"
    MAX_NODES=`lss $OUTDIR/dir-init/*.align | awk 'NR==1{print $NF}' | xargs cat | wc -l`

    #BIGGEST=`grep -c '^_	' $OUTDIR/dir-init/*.align | sed 's/:/ /' | awk '$2==0{print $1}' | sed -e 's/.*shadow-//' -e 's/.align//'`

    #for i
    #do
    #    case "$i" in
    #	*.gw) N=`head -n 10 "$i" | awk 'NF==1&&1*$1==$1&&$1>0{print}'`;;
    #	*.el) N=`newlines < $i | sort -u | wc -l`;;
    #    esac
    #    if echo "$i" | grep -e "$BIGGEST" >/dev/null && [ "$N" -eq "$MAX_NODES" ]; then
    #	BIGGEST_FILE="$i"
    #    fi
    #done
    #[ -f "$BIGGEST_FILE" ] || die "couldn't figure out how to find biggest input network file from '$BIGGEST' and $MAX_NODES"

    mkdir -p $OUTDIR/dir0
    ./scripts/create_shadow.py -s $MAX_NODES -a $OUTDIR/dir-init/shadow-*.align -n "$@" >$OUTDIR/dir0/$NAME-shadow0.el || die "$NAME-shadow0.el network creation failed"
    mv $OUTDIR/dir-init/shadow-*.align $OUTDIR/dir-init/shadow-*.out $OUTDIR/dir0

    # Now get temperature schedule
    mkdir -p $OUTDIR/dir-init
    ls "$@" | awk '{file=$0;gsub(".*/",""); gsub(".el$",""); gsub(".gw$",""); printf "'$SANA' -s3 0 -mec 1 -t 1 -fg1 %s -fg2 '$OUTDIR/dir0/$NAME-shadow0.el' -o '$OUTDIR'/dir-init/%s 2>'$OUTDIR'/dir-init/%s.stderr\n", file,$0,$0}' | parallel_delay networks/$NAME-shadow0/$NAME-shadow0.gw

    grep '^Initial Temperature: ' $OUTDIR/dir-init/*.stderr | sed -e "s,$OUTDIR/dir-init/,," -e 's/\.stderr//' -e 's/:/ /' | awk '{print $1,$4,$7}' > $OUTDIR/dir-init/tinitial-final.txt
    grep '^tdecay' $OUTDIR/dir-init/*.stderr | sed -e "s,$OUTDIR/dir-init/,," -e 's/.stderr//' -e 's/:/ /' | awk '{print $1,$3}' > $OUTDIR/dir-init/tdecay.txt

    echo 'name	tinitial	tfinal	tdecay' | tee $OUTDIR/dir-init/schedule.tsv
    paste $OUTDIR/dir-init/tinitial-final.txt $OUTDIR/dir-init/tdecay.txt | awk '$1==$4{printf "%s\t%s\t%s\t%s\n",$1,$2,$3,$5}' | tee -a $OUTDIR/dir-init/schedule.tsv
fi

for i in `integers $ITER`
do
    echo ---- ITER $i ----- `date`
    i1=`expr $i + 1`
    if [ -f $OUTDIR/dir$i1/$NAME-shadow$i1.el ]; then continue; fi
    mkdir -p $OUTDIR/dir$i1
    for g
    do
	bg=`basename $g .gw`
	bg=`basename $bg .el`
	named-col-prog 'if(!index(name,"'$bg'"))next; i='$i';ITER='$ITER'; e0=log(tinitial);e1=log(tfinal);printf "'$SANA' -s3 0 -mec 1 -t .01 -fg1 '$g' -fg2 '$OUTDIR/dir$i/$NAME-shadow$i.el' -tinitial %g -tdecay %g -o '$OUTDIR/dir$i1/shadow-$bg' 2>'$OUTDIR/dir$i1/shadow-$bg.stderr' -startalignment '$OUTDIR/dir$i/shadow-$bg.out'\n", tinitial*exp((e1-e0)*i/ITER),tdecay/ITER' $OUTDIR/dir-init/schedule.tsv
    done | parallel_delay networks/$NAME-shadow$i/$NAME-shadow$i.gw
    ./scripts/create_shadow.py -c -s $MAX_NODES -a $OUTDIR/dir$i1/shadow-*.out -n "$@" >$OUTDIR/dir$i1/$NAME-shadow$i1.el || die "$OUTDIR/dir$i1/$NAME-shadow$i1.el network creation failed"
done
