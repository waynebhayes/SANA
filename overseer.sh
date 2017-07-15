#!/bin/sh
USAGE="USAGE: $0 sana.exe iterations time-per-iter parallel-spec outdir {list of input networks}
parallel-spec is either a machine file for distrib_stdin, or '-parallel K' for K processes locally"

die() { echo "$USAGE" >&2; echo "FATAL ERROR: $*" >&2; exit 1
}
warn() { echo "Warning: $*" >&2
}

PARALLEL='distrib_stdin.new -s 0.03 -f $MACHINES'

TMPDIR=/tmp/overseer.$$
trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir $TMPDIR

parallel_delay() {
    cat > $TMPDIR/pd
    head -1 $TMPDIR/pd | sh &
    sleep 1
    until [ -f "$1" ]; do sleep 1; done
    tail -n +2 $TMPDIR/pd | eval $PARALLEL
}

SANA="$1"
ITER_EXPR="$2"
T_ITER="$3"
MACHINES="$4"
OUTDIR="$5"
NAME=`basename "$OUTDIR"`
export SANA ITER_EXPR NAME
shift 5

[ -x "$SANA" ] || die "first argument '$SANA' must be an executable file"
case "$MACHINES" in
    -parallel*) PARALLEL='parallel -s bash '`echo $MACHINES | awk '{print $NF}'`;;
    *) [ -f "$MACHINES" ] || die "4th argument '$MACHINES' must be '-parallel N' or a machine list for distrib_stdin";;
esac
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

    # Now get temperature schedule and SES denominator (the latter requires *.out files so don't use -scheduleOnly)
    mkdir -p $OUTDIR/dir-init
    /bin/rm -rf networks/$NAME-shadow0
    ls "$@" | awk '{file=$0;gsub(".*/",""); gsub(".el$",""); gsub(".gw$",""); printf "'$SANA' -t 1 -s3 0 -ses 1 -fg1 %s -fg2 '$OUTDIR/dir0/$NAME-shadow0.el' -o '$OUTDIR'/dir-init/%s 2>'$OUTDIR'/dir-init/%s.stderr\n", file,$0,$0}' | parallel_delay networks/$NAME-shadow0/$NAME-shadow0.gw
    grep '^Initial Temperature: ' $OUTDIR/dir-init/*.stderr | sed -e "s,$OUTDIR/dir-init/,," -e 's/\.stderr//' -e 's/:/ /' | awk '{print $1,$4,$7}' > $OUTDIR/dir-init/tinitial-final.txt
    grep '^tdecay' $OUTDIR/dir-init/*.stderr | sed -e "s,$OUTDIR/dir-init/,," -e 's/.stderr//' -e 's/:/ /' | awk '{print $1,$3}' > $OUTDIR/dir-init/tdecay.txt

    echo 'name	tinitial	tfinal	tdecay' | tee $OUTDIR/dir-init/schedule.tsv
    paste $OUTDIR/dir-init/tinitial-final.txt $OUTDIR/dir-init/tdecay.txt | awk '$1==$4{printf "%s\t%s\t%s\t%s\n",$1,$2,$3,$5}' | tee -a $OUTDIR/dir-init/schedule.tsv
fi
DENOM=`
    cd $OUTDIR/dir-init;
    for i in *.out; do awk '/^G1:/{getline; getline; ;m=$NF;print m}' $i; done |
	sort -n |
	awk '{m[NR-1]=$1}END{for(i=0;i<NR;i++) if(NR-i>1){D+=(NR-i)^2*m[i];for(j=i+1;j<NR;j++)m[j]-=m[i]}; print D}'
`
echo Denominator for ses score is $DENOM
export DENOM
for i in `integers $ITER`
do
    echo ''
    echo -n ---- ITER $i ----- `date`
    i1=`expr $i + 1`
    if [ -f $OUTDIR/dir$i1/$NAME-shadow$i1.el ]; then continue; fi
    mkdir -p $OUTDIR/dir$i1
    for g
    do
	bg=`basename $g .gw`
	bg=`basename $bg .el`
	echo shadow-$bg.out.align >> $OUTDIR/dir$i1/expected-outFiles.txt
	named-col-prog 'if(name!="'$bg'")next; i='$i';ITER='$ITER'; e0=log(tinitial);e1=log(tfinal);printf "'$SANA' -multi-iteration-only -s3 0 -ses 1 -t '$T_ITER' -fg1 '$g' -fg2 '$OUTDIR/dir$i/$NAME-shadow$i.el' -tinitial %g -tdecay %g -o '$OUTDIR/dir$i1/shadow-$bg' 2>'$OUTDIR/dir$i1/shadow-$bg.stderr' -startalignment '$OUTDIR/dir$i/shadow-$bg.out'\n", tinitial*exp((e1-e0)*i/ITER),tdecay/ITER' $OUTDIR/dir-init/schedule.tsv
    done | sort -u > $OUTDIR/dir$i/jobs.txt
    /bin/rm -rf networks/$NAME-shadow$i
    while [ `ls $OUTDIR/dir$i1 | fgrep -f $OUTDIR/dir$i1/expected-outFiles.txt | tee $OUTDIR/dir$i/jobs-done.txt | wc -l` -lt `echo "$@" | wc -w` ]; do
	fgrep -v -f $OUTDIR/dir$i/jobs-done.txt $OUTDIR/dir$i/jobs.txt | parallel_delay networks/$NAME-shadow$i/$NAME-shadow$i.gw # wait until that file is created before parallel'ing
    done
    ./shadow2align.sh $OUTDIR/dir$i1/*.align > $DIR/dir$i1/multiAlign.tsv
    ./scripts/create_shadow.py -c -s $MAX_NODES -a $OUTDIR/dir$i1/shadow-*.out -n "$@" >$OUTDIR/dir$i1/$NAME-shadow$i1.el || die "$OUTDIR/dir$i1/$NAME-shadow$i1.el network creation failed"
    awk '$3>1{sum2+=$3^2}END{printf " SES %g", sum2/'$DENOM'}' $OUTDIR/dir$i1/$NAME-shadow$i1.el
    #./CIQ.sh $OUTDIR/dir$i1/multiAlign.tsv `echo "$@" | newlines | sed 's/\.gw/.el/'` & # don't wait it takes forever
done
