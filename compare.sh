#!/bin/sh
die() { echo "FATAL ERROR: $@" >&2; exit 1
}

SEQ=false
FORCE=false

USAGE="$0 [-seq] [-f] otherAligner g1 g2"
while :; do
    case "$1" in
    -seq) SEQ=true; shift;;
    -f) FORCE=true; shift;;
    -*) die "unknown option '$1'";;
    *) break ;;
    esac
done
M="$1"
G1="$2"
G2="$3"

# Default time parameters
Tcmp=m/9; Tmin=5; Tmax=30

M1=`awk 'NF==1{n++}END{print n}' networks/$G1/$G1.gw`
M2=`awk 'NF==1{n++}END{print n}' networks/$G2/$G2.gw`

if echo $M | grep -q -i '^hubalign$'; then
    Margs=' -s3 0 -importance 1 -objfuntype alpha '
    Sargs=' -s3 0.002 -importance 1 '
elif echo $M | grep -q -i '^l-*graal$'; then
    M=lgraal
    TL=`parse "($M1+$M2)/120"`
    Margs=" -t $TL -s3 0 -graphlet 1 -objfuntype alpha "
    Sargs=' -s3 0 -objfuntype generic -topmeasure wec -wecnodesim graphletlgraal -wec 1 '
elif echo $M | grep -q -i '^mi-*graal$'; then
    M=migraal
    Margs=
    Sargs=' -s3 1 -ec 0 '
elif echo $M | grep -q -i '^magna+*$'; then
    M=magna
    Margs=
    Sargs=''
    Sseq=" -s3 0.05 -sequence 0.9 "
    Mseq=" -s3 0.05 -sequence 0.9 -wrappedArgs \"-d sequence/bitscores/${G1}_${G2}.bitscores -a 0.1 -t 4 \""; # alpha=1 for all topo, 0 for all sequence
elif echo $M | grep -q -i '^natalie$'; then
    Margs=''
    Sargs=' -s3 0 -ec 1 '
    Sseq=" -s3 0.05 -sequence 0.9 "
    Mseq=" -s3 0.05 -sequence 0.9 -wrappedArgs \" -gm sequence/NATALIE/${G1}_${G2}.seqSim -t 600 \"";
elif echo $M | grep -q -i '^optnetalign$'; then
    Tcmp=m/3; Tmin=10; Tmax=60
    Margs="-wrappedArgs \" --timelimit 60 \"";
    Sargs=' -s3 1 -ec 0 '
    Mseq=" -s3 0.05 -sequence 0.9 -wrappedArgs \" --timelimit 60 --nthreads 16 --blastsum --bitscores sequence/wayneScores/${G1}_${G2}.bitscores \"";
    Sseq=" -s3 0.05 -sequence 0.9 "
elif echo $M | grep -q -i '^netal$'; then
    Tcmp='2*m'; Tmin=10; Tmax=30
    Margs=
    Sargs=' -s3 0.4 -ec 0.6 '
elif echo $M | grep -q -i '^gedevo$'; then
    Tcmp=m/9; Tmin=10; Tmax=30
    Margs=" -wrappedArgs \"--maxsecs 36000 --threads 8 \""
    Sargs=' -s3 0.35 -ec 0.65 '
    Mseq=" -wrappedArgs \"--maxsecs 3600 --threads 4 --blastpairlist sequence/bitscores/${G1}-${G2}.bitscores\""
    Sseq=" -ec .5 -s3 0.5 -sequence 0.5 "
elif echo $M | grep -q -i '^wave$'; then
    Margs=" -wrappedArgs sims/graphlet/$G1-$G2.sim "
    Sargs=' -s3 0 -wec 1 -wecnodesim graphlet '
elif echo $M | grep -q -i '^spinal$'; then
    Margs=""
    Sargs=''
    Mseq=" -sequence 1 -ec 1 -alpha 0.5 -wrappedArgs \"-I -ns sequence/SPINAL/${G1}-${G2}\""
    Sseq=" -ec 1 -sequence 300 "
    Tcmp=m; Tmin=3; Tmax=20
elif echo $M | grep -q -i '^piswap$'; then
    Margs=" -wrappedArgs sims/graphlet/$G1-$G2.sim "
    Sargs=''
    Tcmp=m/10; Tmin=10; Tmax=60
elif echo $M | grep -q -i '^ghost$'; then
    Tcmp=m/10; Tmin=10; Tmax=60
    Margs=""
    Sargs='-ec 0.65 -s3 0.35'
else
    die "unknown method $M"
fi

[ -f networks/$G1/$G1.gw ] || die "no such network '$G1'"
[ -f networks/$G2/$G2.gw ] || die "no such network '$G2'"
set -x
if $SEQ; then
    DIR=Mseq
    if $FORCE || [ ! -f $DIR/$M/$G1-$G2.out ]; then
	eval ./sana.$M -method $M $Mseq  -g1 $G1 -g2 $G2 -o $DIR/$M/$G1-$G2 >$DIR/$M/$G1-$G2.stdout 2>$DIR/$M/$G1-$G2.stderr
    fi
else
    DIR=M
    if $FORCE || [ ! -f $DIR/$M/$G1-$G2.out ]; then
	eval ./sana.$M -method $M "$Margs" -g1 $G1 -g2 $G2 -o $DIR/$M/$G1-$G2 >$DIR/$M/$G1-$G2.stdout 2>$DIR/$M/$G1-$G2.stderr
    fi
fi
T="MAX($Tmin,MIN($Tcmp,$Tmax))"
T=`grep -i 'execution time' $DIR/$M/$G1-$G2.out | wawk '{m=$NF/60; print '"$T"'}'`
if $FORCE || [ ! -f $DIR/$M/sana.$G1-$G2.out ]; then
    if $SEQ; then
	eval ./sana.$M -t $T -tinitial auto -tdecay auto $Sseq  -g1 $G1 -g2 $G2 -o $DIR/$M/sana.$G1-$G2 >$DIR/$M/sana.$G1-$G2.stdout 2>$DIR/$M/sana.$G1-$G2.stderr
    else
	./sana.$M -t $T -tinitial auto -tdecay auto $Sargs -g1 $G1 -g2 $G2 -o $DIR/$M/sana.$G1-$G2 >$DIR/$M/sana.$G1-$G2.stdout 2>$DIR/$M/sana.$G1-$G2.stderr
    fi
fi
