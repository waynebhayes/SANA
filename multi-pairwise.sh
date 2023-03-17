#!/bin/bash
USAGE="USAGE: $0 [-frugal] [-archive TYPE] [-H N] [-v|V] {multi-SANA.exe} 'measures' iterations time-per-iter threads outdir {list of input networks}
NECESSARY arguments:
    {multi-SANA.exe}: full path (with leading './' if necessary) to sana.multi executable (usually './sana.multi')
    'measures': one or more multi-alignment measures to optimize--THE LIST MUST BE IN QUOTES
    iterations: number of 'meta-iterations' to perform (usually only tens or hundreds)
    time-per-iter: amount of time to spend on each meta-iterations (in minutes)
    threads: non-negative integer limiting number of processes to run simultaneously; 0 means 'no limit'
    outdir: the output directory where all meta-iteration outputs will be stored
    list-of-input-networks: the list of networks to be aligned
OPTIONS:
    -hide: by default we make the output directory readable on the fly so others can see it progress; -hide turns this off
    -frugal: be frugal with disk space by deleting large unnecessary files after each meta-iteration (eg the shadow network)
    -7z*: means the same as '-archive 7z*', for backwards compatibility
    -archive: upon successful completion, create the specified archive 'outdir.TYPE' and then 'rm -rf outdir'; known TYPEs
	are 7z, 7za, zip, tgz (tar.gz), and tar (no compression).
    -H N: after running simulated annealing, perform N zero-temperature 'hill-climbing' meta-iterations (default zero)
    -v or -V: highly verbose output (only useful for debugging)"

NL='
' # newline
EXEDIR=`dirname $0`
EXEDIR=`cd "$EXEDIR"; /bin/pwd`
PATH="$EXEDIR:$EXEDIR/NetGO:$EXEDIR/scripts:$PATH"
export PATH
echo() { /bin/echo "$@" # shell's built-in echo is broken on MacOS
}

export SANA_TOLERANCE=0 # turn off new default tolerance, and use time / iterations as in old code

MAX_TRIES=3
TIME_VERBOSE=-v
NOBUF='stdbuf -o 0 -e 0'
case `uname` in
Darwin) TIME_VERBOSE=-l
	NOBUF=''
	;;
esac

die() { echo "$USAGE" >&2; echo "FATAL ERROR: $*" >&2; exit 1
}
warn() { echo "Warning: $*" >&2
}
parse() { hawk "BEGIN{print ($*)}" </dev/null
}
newlines() { /bin/awk '{for(i=1; i<=NF; i++) print $i}' "$@"
}
integers() {
    case $# in
    1) awk 'BEGIN{for(i=0;i<'"$1"';i++) print i; exit}' ;;
    2) awk 'BEGIN{for(i='"$1"';i<='"$2"';i++) print i; exit}' ;;
    3) awk 'BEGIN{for(i='"$1"';i<='"$2"';i+='"$3"') print i; exit}' ;;
    esac
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

TMPDIR=/tmp/multi-pairwise.$$
 trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir -p $TMPDIR

VERBOSE=
HILLCLIMB=0
TYPES=false
ARCHIVE=false
ZIP=false
TYPEargs=''
TYPEcreateSh=''
FRUGAL=false
OLD_UMASK=`umask`
CHMOD="chmod -R go+rX"
umask 0022 # read and execute for everybody, but not write
SEScol=4 # becomes 4 if TYPES

while echo "X$1" | grep '^X-' >/dev/null; do
    case "$1" in
    -H) HILLCLIMB=$2; shift;; # in addition to the shift that'll happen below
    -[vV]*) VERBOSE=-v;;
    -7z*) A=`echo $1 | sed 's/^-//'` # get rid of leading dash
	shift; # get rid of -7z* argument
	set DUMMY -archive "$A" "$@"; # DUMMY will get shifted off below, leaving "-archive 7z"
	;;
    -archive) ARCHIVE=true
	case "$2" in
	7z|7za) ZIP="$2"; ZIP_ADD=a; ZIP_EXT=7z; UNZIP="$2"; ZIP_X=x;;
	zip|ZIP) ZIP=zip; ZIP_ADD=''; ZIP_EXT=zip;UNZIP=unzip; ZIP_X='';;
	tar.gz|tgz) ZIP=tar; ZIP_ADD=zcf; ZIP_EXT=tgz;UNZIP=tar; ZIP_X=zxf;;
	tar) ZIP=tar; ZIP_ADD=cf; ZIP_EXT=tar;UNZIP=tar; ZIP_X=xf;;
	*) die "unknown archive type '$2'";;
	esac
	shift; # in addition to the shift below
	echo "Using archiver '$ZIP' with add option '$ZIP_ADD' and filename extension '$ZIP_EXT'"
	;;
    -frugal) FRUGAL=true;;
    -hide) CHMOD=:; umask $OLD_UMASK;;
    -*) die "unknown option '$1'";;
    esac
    shift
done

SANA_MULTI="$1"
MEASURES="$2"
ITER_EXPR="$3"
T_ITER="$4"
THREADS="$5"
OUTDIR="$6"
NAME=`basename "$OUTDIR"`
export SANA_MULTI ITER_EXPR NAME
shift 6

[ -x "$SANA_MULTI" ] || die "first argument '$SANA_MULTI' must be an executable file"

# Ensure "$THREADS" is an integer:
[ `expr "$THREADS" '*' 1` -eq "$THREADS" ] || die "threads '$THREADS' must be a non-negative integer"
# Ensure it's >= 0
[ $THREADS -ge 0 ] || die "threads '$THREADS' must be a non-negative integer"

PARALLEL_EXE=${PARALLEL_EXE:="$EXEDIR/parallel"}
if head -1 "$PARALLEL_EXE" | grep -a 'exec bash' >/dev/null; then
    warn "parallel executable is bash; setting theads to 1"
    THREADS=1
    PARALLEL_CMD="$PARALLEL_EXE $THREADS"
else
    case "$THREADS" in
	0) PARALLEL_CMD="$PARALLEL_EXE -s bash $#";; # the number of networks
	*) PARALLEL_CMD="$PARALLEL_EXE -s bash $THREADS";; # number of threads user specifies
    esac
fi
NUM_ITERS=`parse "$ITER_EXPR"` || die "'$ITER_EXPR': cannot figure out iteration count"
if [ -d "$OUTDIR/.init" ]; then
    warn "outdir '$OUTDIR' already exists; continuing"
fi
DIR_DIGITS=`echo -n "$NUM_ITERS" | wc -c | sed 's/[^0-9]//g'` # num digits for "hundreds" directory names
DirDigits() { # DirDigits is expect an integer on its STANDARD INPUT, not command line!
    hawk '{ASSERT(ARGC==1 && NR==1 && NF==1 && $1*1==$1 && $1>=0);
	printf "%0'$DIR_DIGITS'ds/%02d\n", 100*int($1/100), $1 % 100}' "$@"
}

mkdir -p $OUTDIR/.init || die "Cannot make outdir '$OUTDIR'"
$CHMOD $OUTDIR

# NOTE: REMAINDER OF THE COMMAND LINE IS ALL THE INPUT NETWORKS
# We want them to be in alphabetical order
set `/bin/ls "$@"`
export NUM_GRAPHS=$#

#script to create an fshadow config file for SANA's mode shadow
ShadowConfig() {
    #USAGE: shadowOutFile k {k graphs} alignFiles? [ maybe k alignFiles ] colors? [ maybe: numColors + "name dummyCount" pairs + k colorFiles + shadowColorOutFile]
    shadowFile="$1"; shift
    numGraphs="$1"; shift
    k=$numGraphs
    GRAPHS=""
    while [ "$k" -gt 0 ]; do
	GRAPHS="$GRAPHS$1$NL"; shift; k=`expr $k - 1 `
    done
    alignFiles=$1; shift
    ALIGN_FILES=""
    if [ $alignFiles -ne 0 ]; then
	k=$numGraphs
	while [ "$k" -gt 0 ]; do
	    ALIGN_FILES="$ALIGN_FILES$1$NL"; shift; k=`expr $k - 1 `
	done
    fi
    colors="$1"; shift
    if [ "$colors" = 0 ]; then
	numColors=1
	colorFiles=0
	COLOR_FILES=""
	COLOR_DUMMYCOUNT="__default 100"
    else
	numColors=$1; shift
	colorFiles=1
	k=$numColors
	while [ "$k" -gt 0 ]; do
	    COLOR_DUMMYCOUNT="$COLOR_DUMMYCOUNT$1$NL"; shift; k=`expr $k - 1 `
	done
	k=$numGraphs
	while [ "$k" -ge 0 ]; do # ge because we also need the color outfile last
	    COLOR_FILES="$COLOR_FILES$1$NL"; shift; k=`expr $k - 1 `
	done
    fi

cat <<-EOF
	$shadowFile # file where the created shadow is stored
	$numGraphs # numGraphs plus the graph files
	$GRAPHS
	
	$alignFiles # 0/1 indicating if alignment files are provided, followed by k files if so
	$ALIGN_FILES
	
	$colorFiles # 0/1 indicating if color files are provided, followed by k files if so, and a file name to save the shadow colors
	$COLOR_FILES
	
	# number C of colors, followed by the C color names and their number of dummy nodes
	$numColors                                              
	# (if not providing color files, the number of colors is 1 and the number of dummy nodes
	# must be specified for __default)
	$COLOR_DUMMYCOUNT
	EOF
}

ZERO_DIR=`echo 0 | DirDigits`
SHADOW_GRAPH="$OUTDIR/$ZERO_DIR/$NAME-shadow0.gw"
# Create initial random alignment, which also tells us the number of nodes.
# When checking to continue, do NOT check for the .gw file since it may be gone due to -frugal
if [ -f "$OUTDIR/.init/group.multiAlign" ] && [ `ls $OUTDIR/$ZERO_DIR/*-shadow.align 2>/dev/null | wc -l` -eq $NUM_GRAPHS ]
then
    echo "not creating initial alignment since we're continuing"
else
    mkdir -p $OUTDIR/$ZERO_DIR
    $CHMOD $OUTDIR/$ZERO_DIR
    ./random-multi-alignment.sh $TYPES "$OUTDIR"/.init "$@"
    ShadowConfig $SHADOW_GRAPH $# "$@" 1 $OUTDIR/.init/*-shadow.align 0 > $OUTDIR/$ZERO_DIR/fshadow.txt
    # "-it 1" means perform 1 token iteration, since 0 is no longer allowed
    $SANA_MULTI -tolerance 0 -it 1 -mode shadow -fshadow $OUTDIR/$ZERO_DIR/fshadow.txt > $OUTDIR/$ZERO_DIR/fshadow.stdout 2>$OUTDIR/$ZERO_DIR/fshadow.stderr
    mv $OUTDIR/.init/*-shadow.align $OUTDIR/$ZERO_DIR
fi

echo -n "Computing SES denominator..."
export SES_DENOM=`numEdges "$@" | sort -n | awk '{m[NR-1]=$1}END{for(i=0;i<NR;i++) if(NR-i>=1){D+=(NR-i)^2*m[i];for(j=i+1;j<NR;j++)m[j]-=m[i]}; print D}'`
ALL_EDGES=`numEdges "$@" | sort -n`
export EDGE_SUM=`echo "$ALL_EDGES" | add`
export MAX_EDGE=`echo "$ALL_EDGES" | tail -1`
echo Denominator for SES score is $SES_DENOM, EDGE_SUM is $EDGE_SUM, MAX_EDGE is $MAX_EDGE
# Now get temperature schedule and SES denominator (the latter requires *.out files so don't use -scheduleOnly)
mkdir -p $OUTDIR/.init
$CHMOD $OUTDIR/.init
/bin/rm -rf networks/$NAME-shadow0
touch $OUTDIR/.init/schedule.tsv $OUTDIR/.init/tdecay.txt
TIME_LIMIT=`parse "60*($T_ITER+20)"`
TIME_LIMIT2=`parse "20*($TIME_LIMIT)"`
$CHMOD $OUTDIR
if true; then # hard-coded as "true" but can be made "false" for debugging purposes
    TRIES=0
    while [ `awk '{printf "%s.stdout\n", $1}' $OUTDIR/.init/schedule.tsv | tee $OUTDIR/.init/schedule.done | wc -l` -lt `echo name "$@" | wc -w` ]; do
	[ "$TRIES" -ge $MAX_TRIES ] && die "tried $TRIES times and failed to compute temperature schedule"
	echo "Computing temperature endpoints... (attempt #$TRIES)"
	/bin/rm -rf networks/$NAME-shadow0
	mkdir    -p networks/$NAME-shadow0; (cd networks/$NAME-shadow0; ln -s $TMPDIR/autogenerated .)
	ls "$@" | awk '{file=$0;gsub(".*/",""); gsub(".el$",""); gsub(".gw$",""); printf "mkdir -p '$OUTDIR/.init';'"/usr/bin/time $TIME_VERBOSE $SANA_MULTI $MEASURES $TYPEargs"' -multi-iteration-only -tolerance 0 -it 1 -fg1 %s -fg2 '$SHADOW_GRAPH' -startalignment '$OUTDIR/$ZERO_DIR'/%s-shadow.align -o '$OUTDIR'/.init/%s >'$OUTDIR'/.init/%s.stdout 2>'$OUTDIR'/.init/%s.stderr\n", file,$0,$0,$0,$0}' | fgrep -v -f $OUTDIR/.init/schedule.done | tee $OUTDIR/.init/jobs.txt | eval $PARALLEL_CMD
	awk '/^Computed TInitial .* in .*s$/{Tinit[FILENAME]=$3}/^Computed TFinal .* in .*s$/{Tfinal[FILENAME]=$3}/^TDecay needed to traverse this range:/{Tdecay[FILENAME]=$NF}END{for(i in Tinit)print i,Tinit[i],Tfinal[i],Tdecay[i]}' $OUTDIR/.init/*.stdout | sed -e "s,$OUTDIR/.init/,," -e 's/\.stdout//' > $OUTDIR/.init/tinitial-final.txt
	echo 'name	tinitial	tfinal	tdecay' | tee $OUTDIR/.init/schedule.tsv
	sed 's/ /	/g' $OUTDIR/.init/tinitial-final.txt | tee -a $OUTDIR/.init/schedule.tsv
	$CHMOD $OUTDIR
	TRIES=`expr $TRIES + 1`
    done
    STDOUT_PATTERN="$OUTDIR/.init/*.stdout"
    STDOUT_FILES=( $STDOUT_PATTERN )
    if grep -q 'Increasing number of samples for linear regression' ${STDOUT_FILES[0]} ; then
        echo 'WARNING: temerature search interval ends computed to be identical.';
    fi
else
    echo 'name	tinitial	tfinal	tdecay' | tee $OUTDIR/.init/schedule.tsv
    ls "$@" | awk '{file=$0;gsub(".*/",""); gsub(".el$",""); gsub(".gw$","");printf "%s	40	1e-10	5\n",$1}' | tee -a $OUTDIR/.init/schedule.tsv
fi
$CHMOD $OUTDIR
case `hostname` in
[Jj]enkins*) echo -n "Note: Jenkins will only show every 10 iterations";;
esac
for ITER in `integers $NUM_ITERS` `integers $NUM_ITERS $HILLCLIMB`
do
    ITER_dir=`echo $ITER | DirDigits`
    i1_dir=`expr $ITER + 1 | DirDigits`
    /bin/rm -rf networks/*/autogenerated/*
    if [ "$VERBOSE" = "" ]; then
	if [ `echo "$ITER % 10" | bc` -eq 0 ]; then echo ""; echo -n "ITER:"; fi
	$NOBUF echo -n " $ITER"
    else
	$NOBUF echo -n ---- ITER $ITER -----
    fi
    if [ $ITER -ge $NUM_ITERS ]; then echo -n " (HillClimb)"; fi
    i1=`expr $ITER + 1`
    if [ -f  $OUTDIR/$i1_dir/jobs-done.txt ]; then
       if [ `wc -l < $OUTDIR/$i1_dir/jobs-done.txt` -eq `echo "$@" | wc -w` ]; then
	    continue
	fi
    fi
    mkdir -p   $OUTDIR/$i1_dir
    $CHMOD     $OUTDIR/$i1_dir
    /bin/rm -f $OUTDIR/$i1_dir/expected-outFiles.txt
    SHADOW_GRAPH="$OUTDIR/$ITER_dir/$NAME-shadow$ITER.gw"
    for g
    do
	bg=`basename $g .gw`
	bg=`basename $bg .el`
	echo $bg-shadow.align >> $OUTDIR/$i1_dir/expected-outFiles.txt
	# compute iterations to perform based on the iterations-per-second computed in init for this network
	SA_ITERS=`awk '/ ips = /&&!/ inf/{for(i=1;i<NF;i++)if($i=="ips"){n++;ipsSum+=$(i+2)}}END{printf "%d\n", '$T_ITER'*60*ipsSum/n}' $OUTDIR/.init/$bg.stdout`
	awkcel '{if(name!="'$bg'")next; ITER='$ITER';NUM_ITERS='$NUM_ITERS'; e0=log(tinitial);e1=log(tfinal);printf "'"mkdir -p $OUTDIR/$i1_dir; /bin/rm -f networks/$NAME-shadow$ITER/autogenerated/$NAME-shadow${ITER}_UntypedNodes_Unlocked_Weighted.bin; /usr/bin/time $TIME_VERBOSE $SANA_MULTI $MEASURES $TYPEargs -multi-iteration-only -tolerance 0 -it $SA_ITERS -fg1 $g -fg2 $SHADOW_GRAPH -tinitial %g -tdecay %g -startalignment $OUTDIR/$ITER_dir/$bg-shadow.align -o $OUTDIR/$i1_dir/$bg-shadow >$OUTDIR/$i1_dir/$bg-shadow.stdout 2>$OUTDIR/$i1_dir/$bg-shadow.stderr"'", tinitial*exp((e1-e0)*ITER/NUM_ITERS),tdecay/NUM_ITERS;print ""}' $OUTDIR/.init/schedule.tsv
    done | sort -u > $OUTDIR/$ITER_dir/jobs.txt
    $CHMOD $OUTDIR/$ITER_dir $OUTDIR/$i1_dir
    /bin/rm -rf networks/$NAME-shadow$ITER
    TRIES=0
    while [ `ls $OUTDIR/$i1_dir | fgrep -f $OUTDIR/$i1_dir/expected-outFiles.txt | tee $OUTDIR/$ITER_dir/jobs-done.txt | wc -l` -lt `echo "$@" | wc -w` ]; do
	[ "$TRIES" -ge $MAX_TRIES ] && die "tried $TRIES times and failed to advance ITER from $ITER"
	TRIES=`expr $TRIES + 1`
        /bin/rm -rf networks/*/autogenerated/*
	$CHMOD $OUTDIR/$ITER_dir $OUTDIR/$i1_dir
        sed -e 's/\.out//' -e 's/\.align/.stdout/' $OUTDIR/$ITER_dir/jobs-done.txt | fgrep -v -f /dev/stdin $OUTDIR/$ITER_dir/jobs.txt | eval $PARALLEL_CMD
	$CHMOD $OUTDIR/$ITER_dir $OUTDIR/$i1_dir
    done
    ./shadow2align.sh $OUTDIR/$i1_dir/*.align > $OUTDIR/$i1_dir/multiAlign.tsv
    $CHMOD $OUTDIR/$ITER_dir $OUTDIR/$i1_dir
    SHADOW_GRAPHi1="$OUTDIR/$i1_dir/$NAME-shadow$i1.gw"
    ShadowConfig "$SHADOW_GRAPHi1" $# "$@" 1 $OUTDIR/$i1_dir/*-shadow.align 0 > $OUTDIR/$i1_dir/fshadow.txt
    $SANA_MULTI -tolerance 0 -it 1 -mode shadow -fshadow $OUTDIR/$i1_dir/fshadow.txt > $OUTDIR/$i1_dir/fshadow.stdout 2> $OUTDIR/$i1_dir/fshadow.stderr || die "$OUTDIR/$i1_dir/$NAME-shadow$i1.gw network creation failed"
#   awk '{gsub("[|{}]","")}$'$SEScol'>1{sum2+=$'$SEScol'^2}END{printf " SES %g\n", sum2/'$SES_DENOM'}' $OUTDIR/$i1_dir/$NAME-shadow$i1.gw
    $CHMOD $OUTDIR/$ITER_dir $OUTDIR/$i1_dir
    if $FRUGAL; then /bin/rm -f $SHADOW_GRAPH; fi # this file is BY FAR the largest, per directory, so nuke it
done
[ "$VERBOSE" = "" ] && echo "" # final newline

if $ARCHIVE; then
    echo "Archiving to '$OUTDIR.$ZIP_EXT'"
    DO=`dirname "$OUTDIR"`
    BO=`basename "$OUTDIR"`
    (set -x; cd "$DO" && eval $ZIP $ZIP_ADD "$BO.$ZIP_EXT" "$BO" && /bin/rm -rf "$BO"; set +x)
fi
chmod -R go+rX $OUTDIR*

#echo "Computing CIQ... may take awhile..."
#/CIQ.sh $OUTDIR/$i1_dir/multiAlign.tsv `echo "$@" | newlines | sed 's/\.gw/.el/'`

