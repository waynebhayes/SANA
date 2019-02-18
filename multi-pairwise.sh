#!/bin/sh
USAGE="USAGE: $0 [-H N] sana.exe 'measures' iterations time-per-iter parallel-spec outdir {list of input networks}
parallel-spec is either a machine file for distrib_stdin, or '-parallel K' for K processes locally"

die() { echo "$USAGE" >&2; echo "FATAL ERROR: $*" >&2; exit 1
}
warn() { echo "Warning: $*" >&2
}
parse() { awk "BEGIN{print ($*)}" </dev/null
}
newlines() { /bin/awk '{for(i=1; i<=NF; i++) print $i}' "$@"
}
parallel() { /home/wayne/bin/bin.x86_64/parallel "$@"
}
lss() {
    LS=/bin/ls; SORT="sort -k +5nr"
    #LS=/usr/ucb/ls; SORT="sort +3nr"

    # List filenames in order of non-increasing byte size.
    # Don't descend directories unless the files in them are explicitly listed.

    # This rigamarole needs to be here.  Delete at your own risk.
    # The loop is fast; no external commands are called.

    # We can't just do "ls -ld $*" because if there are no arguments, all we get is
    #        drwx------   4 wayne        1024 Sep 24 15:46 .
    # which isn't what we intended.  Thus, if there are no arguments, we list
    # everything in the current directory; otherwise, list everything that's
    # listed on the command line, but don't descend any directories.  But now
    # we need to recognize if no *files* were listed, but options to ls(1) were
    # listed.  So we have to erase all the options before asking "was there
    # anything on the command line?"  The loop is fast; no external commands
    # are called.  Finally, we need to shift away the options and use "$@"
    # to pass the list of files, in case the filenames have spaces in them.

    #set -- `getopt aAbcFLnpqu "$@"`

    accept_opts=aAbcFLnpqul
    files=N
    opts=
    while :; do
	if [ "$#" = 0 ]; then 
	    break
	fi
	case "$1" in
	    -*) if getopt $accept_opts "$1" >/dev/null; then
		    opts="$opts $1"
		    shift
		else
		    # getopt prints the error message for us
		    exit 1
		fi
		;;
	    --) break;;
	    *)  files=Y
		break   # files begin here
		;;
	esac
    done

    case "$files" in
	N) $LS -l $opts | $SORT ;;
	Y) $LS -ld $opts "$@" | $SORT ;;
    esac
}

export PARALLEL='distrib_sge "$JOB-$ITER" "$OUTDIR/dir$ITER"'

TMPDIR=/tmp/overseer.$$
trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir $TMPDIR

parallel_delay() {
    eval $PARALLEL
    #cat > $TMPDIR/pd
    #head -1 $TMPDIR/pd | tee /dev/tty | sh &
    #sleep 1
    #until [ -f "$1" ]; do sleep 1; done
    #tail -n +2 $TMPDIR/pd | tee /dev/tty | eval $PARALLEL
}

HILLCLIMB=0
TYPES=false
TYPEargs=''
TYPEcreateSh=''
CrShOutType=gw # becomes el if TYPES
CrShAlignInput=out # becomes align if TYPES
SEScol=4 # becomes 4 if TYPES
while echo "X$1" | grep '^X-' >/dev/null; do
    case "$1" in
    -H) HILLCLIMB=$2; shift;; # in addition to the shift that'll happen below
    -nodes-have-types) TYPES=true; TYPEargs='-nodes-have-types -lock-same-names'; SEScol=3;
	CrShOutType=el; CrShAlignInput=align; TYPEcreateSh='--nodes-have-types --shadowNames Jurisica/SHADOW.all.txt';
	;;
    -*) die "unknown option '$1'";;
    esac
    shift
done
SANA="$1"
MEASURES="$2"
ITER_EXPR="$3"
T_ITER="$4"
JOB="$5"
OUTDIR="$6"
NAME=`basename "$OUTDIR"`
export SANA ITER_EXPR NAME
shift 6

[ -x "$SANA" ] || die "first argument '$SANA' must be an executable file"

case "$JOB" in
    -parallel*) PARALLEL='parallel -s bash '`echo $JOB | awk '{print $NF}'`;;
    *) ;; #[ -f "$JOB" ] || die "4th argument '$JOB' must be '-parallel N' or a job name for distrib_sge";;
esac
NUM_ITERS=`parse "$ITER_EXPR"` || die "'$ITER_EXPR': cannot figure out iteration count"
if [ -d "$OUTDIR" ]; then
    warn "outdir '$OUTDIR' already exists; continuing"
fi

mkdir -p $OUTDIR/dir000 $OUTDIR/networks || die "Cannot make outdir '$OUTDIR'"

# NOTE: REMAINDER OF THE COMMAND LINE IS ALL THE INPUT NETWORKS
# We want them to be in alphabetical order
set `/bin/ls "$@"`

# Create initial random alignment, which also tells us the number of nodes.
[ -f "$OUTDIR"/dir000/group.multiAlign ] || ./random-multi-alignment.sh $TYPES "$OUTDIR"/dir000 "$@"
MAX_NODES=`wc -l < "$OUTDIR"/dir000/group.multiAlign`
MAX_NODES=`parse "int($MAX_NODES+1)"`

mkdir -p $OUTDIR/dir0
# NOTE: do we want to use *-shadow.out, or *-shadow.align? That's how the next two lines differ
 [ -f $OUTDIR/dir0/$NAME-shadow0.$CrShOutType ] || (./createShadow -s$MAX_NODES $TYPEcreateSh "$@" $OUTDIR/dir000/*-shadow.$CrShAlignInput >$OUTDIR/dir0/$NAME-shadow0.$CrShOutType) || die "$NAME-shadow0.$CrShOutType network creation failed"
#[ -f $OUTDIR/dir0/$NAME-shadow0.$CrShOutType ] || (./createShadow -s29315      $TYPEcreateSh "$@" $OUTDIR/dir000/*-shadow.$CrShAlignInput >$OUTDIR/dir0/$NAME-shadow0.$CrShOutType) || die "$NAME-shadow0.$CrShOutType network creation failed"

# Add 10 dummy "holes" for mRNAs at the end of the shadow network, each with weight 0 edges.
[ $TYPES ] && yes | head -26 | (awk '{printf "DUMMY\tmRNA-%c\t0\n", 64+NR}' >> $OUTDIR/dir0/$NAME-shadow0.el)
mv $OUTDIR/dir000/*-shadow.align $OUTDIR/dir000/*-shadow.out $OUTDIR/dir0

echo -n "Computing SES denominator..."
export SES_DENOM=`numEdges "$@" | sort -n | awk '{m[NR-1]=$1}END{for(i=0;i<NR;i++) if(NR-i>=1){D+=(NR-i)^2*m[i];for(j=i+1;j<NR;j++)m[j]-=m[i]}; print D}'`
ALL_EDGES=`numEdges "$@" | sort -n`
export EDGE_SUM=`echo "$ALL_EDGES" | add`
export MAX_EDGE=`echo "$ALL_EDGES" | tail -1`
echo Denominator for SES score is $SES_DENOM, EDGE_SUM is $EDGE_SUM, MAX_EDGE is $MAX_EDGE
# Now get temperature schedule and SES denominator (the latter requires *.out files so don't use -scheduleOnly)
mkdir -p $OUTDIR/dir000
/bin/rm -rf networks/$NAME-shadow0
touch $OUTDIR/dir000/schedule.tsv $OUTDIR/dir000/tdecay.txt
TIME_LIMIT=`parse "60*($T_ITER+20)"`
TIME_LIMIT2=`parse "20*($TIME_LIMIT)"`
if true; then
    ITER=000
    while [ `awk '{printf "%s.stdout\n", $1}' $OUTDIR/dir000/schedule.tsv | tee $OUTDIR/dir000/schedule.done | wc -l` -lt `echo name "$@" | wc -w` ]; do
	/bin/rm -rf networks/$NAME-shadow0
	mkdir    -p networks/$NAME-shadow0; (cd networks/$NAME-shadow0; ln -s /tmp/autogenerated .)
	ls "$@" | awk '{file=$0;gsub(".*/",""); gsub(".el$",""); gsub(".gw$",""); printf "mkdir -p '$OUTDIR/dir000';'"/usr/bin/time -v timed-run -$TIME_LIMIT $SANA $MEASURES $TYPEargs"' -multi-iteration-only -t 0 -fg1 %s -fg2 '$OUTDIR/dir0/$NAME-shadow0.$CrShOutType' -startalignment '$OUTDIR'/dir0/%s-shadow.align -o '$OUTDIR'/dir000/%s >'$OUTDIR'/dir000/%s.stdout 2>'$OUTDIR'/dir000/%s.stderr\n", file,$0,$0,$0,$0}' | fgrep -v -f $OUTDIR/dir000/schedule.done | tee $OUTDIR/dir000/jobs.txt | eval timed-run -$TIME_LIMIT2 $PARALLEL #parallel_delay networks/$NAME-shadow0/autogenerated/$NAME-shadow0_NodeNameToIndexMap.bin
	awk '/^Initial temperature/{Tinit[FILENAME]=$4}/^Final temperature /{Tfinal[FILENAME]=$4}/^tdecay:/{Tdecay[FILENAME]=$2}END{for(i in Tinit)print i,Tinit[i],Tfinal[i],Tdecay[i]}' $OUTDIR/dir000/*.stdout | sed -e "s,$OUTDIR/dir000/,," -e 's/\.stdout//' > $OUTDIR/dir000/tinitial-final.txt
	echo 'name	tinitial	tfinal	tdecay' | tee $OUTDIR/dir000/schedule.tsv
	sed 's/ /	/g' $OUTDIR/dir000/tinitial-final.txt | tee -a $OUTDIR/dir000/schedule.tsv
    done
else
    echo 'name	tinitial	tfinal	tdecay' | tee $OUTDIR/dir000/schedule.tsv
    ls "$@" | awk '{file=$0;gsub(".*/",""); gsub(".el$",""); gsub(".gw$","");printf "%s	40	1e-10	5\n",$1}' | tee -a $OUTDIR/dir000/schedule.tsv
fi
for ITER in `integers $NUM_ITERS` `integers $NUM_ITERS $HILLCLIMB`
do
    /bin/rm -rf networks/*/autogenerated/*
    echo -n ---- ITER $ITER -----
    if [ $ITER -ge $NUM_ITERS ]; then echo -n "(HillClimb) "; T_ITER=1; fi
    i1=`expr $ITER + 1`
    if [ -f  $OUTDIR/dir$i1/$NAME-shadow$i1.$CrShOutType ]; then continue; fi
    if [ -f  $OUTDIR/dir$i1/jobs-done.txt ]; then
       if [ `wc -l < $OUTDIR/dir$i1/jobs-done.txt` -eq `echo "$@" | wc -w` ]; then
	    continue
	fi
    fi
    mkdir -p   $OUTDIR/dir$i1
    /bin/rm -f $OUTDIR/dir$i1/expected-outFiles.txt
    for g
    do
	bg=`basename $g .gw`
	bg=`basename $bg .el`
	echo $bg-shadow.align >> $OUTDIR/dir$i1/expected-outFiles.txt
	named-col-prog 'if(name!="'$bg'")next; ITER='$ITER';NUM_ITERS='$NUM_ITERS'; e0=log(tinitial);e1=log(tfinal);printf "mkdir -p '$OUTDIR/dir$i1';'"/bin/rm -f networks/$NAME-shadow$ITER/autogenerated/$NAME-shadow${ITER}_UntypedNodes_Unlocked_Weighted.bin; /usr/bin/time -v timed-run -$TIME_LIMIT $SANA $MEASURES $TYPEargs"' -multi-iteration-only -t '$T_ITER' -fg1 '$g' -fg2 '$OUTDIR/dir$ITER/$NAME-shadow$ITER.$CrShOutType' -tinitial %g -tdecay %g -o '"$OUTDIR/dir$i1/$bg-shadow >$OUTDIR/dir$i1/$bg-shadow.stdout 2>$OUTDIR/dir$i1/$bg-shadow.stderr -startalignment $OUTDIR/dir$ITER/$bg-shadow.align"'\n", tinitial*exp((e1-e0)*ITER/NUM_ITERS),tdecay/NUM_ITERS' $OUTDIR/dir000/schedule.tsv
    done | sort -u > $OUTDIR/dir$ITER/jobs.txt
    /bin/rm -rf networks/$NAME-shadow$ITER
    ln -s $OUTDIR/networks networks/$NAME-shadow$ITER; (cd networks/$NAME-shadow$ITER; ln -s /tmp/autogenerated .)
    while [ `ls $OUTDIR/dir$i1 | fgrep -f $OUTDIR/dir$i1/expected-outFiles.txt | tee $OUTDIR/dir$ITER/jobs-done.txt | wc -l` -lt `echo "$@" | wc -w` ]; do
	/bin/rm -rf networks/*/autogenerated/*
	sed -e 's/\.out//' -e 's/\.align/.stdout/' $OUTDIR/dir$ITER/jobs-done.txt | fgrep -v -f - $OUTDIR/dir$ITER/jobs.txt | eval timed-run -$TIME_LIMIT2 $PARALLEL #parallel_delay networks/$NAME-shadow$ITER/autogenerated/$NAME-shadow${i}_NodeNameToIndexMap.bin || # wait until that directory is created before parallel'ing
    done
    ./shadow2align.sh $OUTDIR/dir$i1/*.align > $OUTDIR/dir$i1/multiAlign.tsv
     ./createShadow -s$MAX_NODES $TYPEcreateSh "$@" $OUTDIR/dir$i1/*-shadow.$CrShAlignInput  >$OUTDIR/dir$i1/$NAME-shadow$i1.$CrShOutType || die "$OUTDIR/dir$i1/$NAME-shadow$i1.$CrShOutType network creation failed"
    #./createShadow -s29315      $TYPEcreateSh "$@" $OUTDIR/dir$i1/*-shadow.$CrShAlignInput > $OUTDIR/dir$i1/$NAME-shadow$i1.$CrShOutType || die "$OUTDIR/dir$i1/$NAME-shadow$i1.$CrShOutType network creation failed"
    [ $TYPES ] && yes | head | (awk '{printf "DUMMY\tmRNA-%c\t0\n", 64+NR}' >> $OUTDIR/dir$i1/$NAME-shadow$i1.el)
#   awk '{gsub("[|{}]","")}$'$SEScol'>1{sum2+=$'$SEScol'^2}END{printf " SES %g\n", sum2/'$SES_DENOM'}' $OUTDIR/dir$i1/$NAME-shadow$i1.$CrShOutType
done
echo "Computing CIQ... may take awhile..."
#/CIQ.sh $OUTDIR/dir$i1/multiAlign.tsv `echo "$@" | newlines | sed 's/\.gw/.el/'`
