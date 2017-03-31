#!/bin/sh
die() { echo "$USAGE" >&2; echo "FATAL ERROR: $@" >&2; exit 1
}

SEQ=false
FORCE=false

USAGE="USAGE: $0 [-seq <ialpha_digit>] [-f] otherAligner g1 g2"
ialpha=0
while :; do
    case "$1" in
    -seq) SEQ=true;
	case "$2" in
	[0-9]) ialpha=$2; shift 2;;
	*) die "-seq <digit> where ialpha is an integer 0-9 representing alpha=0.<digit>";;
	esac
	;;
    -f) FORCE=true; shift;;
    -*) die "unknown option '$1'";;
    *) break ;;
    esac
done
M="$1"
G1="$2"
G2="$3"
[ -f networks/$G1/$G1.gw ] || die "no such network '$G1'"
[ -f networks/$G2/$G2.gw ] || die "no such network '$G2'"

# Default time parameters
Tcmp=m/9; Tmin=5; Tmax=20

M1=`awk 'NF==1{n++}END{print n}' networks/$G1/$G1.gw`
M2=`awk 'NF==1{n++}END{print n}' networks/$G2/$G2.gw`

if echo $M | grep -q -i '^hubalign$'; then
    Tcmp=m; Tmin=3; Tmax=20
    Margs=' -s3 0 -importance 1 -objfuntype alpha '
    Sargs=' -s3 0.002 -importance 1 '
    Mseq=" -s3 0 -objfuntype alpha -alpha 0.$ialpha -sequence 0.$ialpha "
    alpha=`parse "0.9+0.$ialpha/10"`
    beta=`parse "1-$alpha"`
    Sseq=" -s3 $beta -importance 0.0 -sequence $alpha"
elif echo $M | grep -q -i '^l-*graal$'; then
    M=lgraal
    TL=`parse "($M1+$M2)/120"`
    Margs=" -t $TL -s3 0 -graphlet 1 -objfuntype alpha "
    Sargs=' -s3 0 -objfuntype generic -topmeasure wec -wecnodesim graphletlgraal -wec 1 '
    Mseq=" -t $TL -s3 0 -graphlet 1 -objfuntype alpha -alpha 0.$ialpha -sequence 0.$ialpha"
    Sseq=" -s3 0 -objfuntype alpha -topmeasure wec -wecnodesim graphletlgraal -wec 1 -alpha 0.$ialpha -sequence 0.$ialpha"
elif echo $M | grep -q -i '^mi-*graal$'; then
    M=migraal
    Margs="-wrappedArgs '-p 1'"
    Sargs=' -s3 1 -ec 0 '
    Mseq="-wrappedArgs '-p 17 -q sequence/bitscore/${G1}_${G2}.bitscore'"
    Sseq=' -s3 1 -ec 0 -sequence 1'
elif echo $M | grep -q -i '^magna+*$'; then
    M=magna
    Margs=
    Sargs=''
    beta=`parse "1-0.$ialpha"`
    Mseq=" -s3 $beta -sequence 0.$ialpha -wrappedArgs \"-d sequence/bitscores/${G1}_${G2}.bitscores -a $beta -t 4 \""; # our beta is their -a=1 for all topo, 0 for all sequence
    Sseq=" -s3 $beta -sequence 0.$ialpha "
elif echo $M | grep -q -i '^natalie$'; then
    Margs=" -wrappedArgs \" -gm ../../sequence/evalue/${G1}_${G2}_blast.out -t 30000 -beta 1 -sf 5 \""; # beta 1 means topo only, sf5=EC
    Sargs=' -s3 0 -ec 1 '
    beta=`parse "1-0.$ialpha"`
    Sseq=" -s3 0.05 -sequence 0.9 "
    Mseq=" -s3 0.05 -sequence 0.9 -wrappedArgs \" -gm ../../sequence/evalue/${G1}_${G2}_blast.out -t 30000 -beta $beta \""
elif echo $M | grep -q -i '^optnetalign$'; then
    Tcmp=m/3; Tmin=3; Tmax=20
    Margs="-wrappedArgs \" --timelimit 60 \"";
    Sargs=' -s3 1 -ec 0 '
    beta=`parse "1-0.$ialpha"`
    Mseq=" -s3 $beta -sequence 0.$ialpha -alpha 0.$ialpha -wrappedArgs \" --timelimit 180 --nthreads 8 --blastsum --bitscores sequence/wayneScores/${G1}_${G2}.bitscores \"";
    Sseq=" -s3 $beta -sequence 0.$ialpha -alpha 0.$ialpha "
elif echo $M | grep -q -i '^netal$'; then
    Tcmp='2*m'; Tmin=10; Tmax=30
    Margs=
    Sargs=' -s3 0.4 -ec 0.6 '
    Mseq="-sequence 0.$ialpha -ec .1 -wrappedArgs \"-b 0.$ialpha\""
    Sseq='-s3 0.4 -ec 0.6 -sequence 1'
elif echo $M | grep -q -i '^cytogedevo$'; then
    Tcmp=m/3; Tmin=3; Tmax=30
    Margs=" -wrappedArgs \"--maxsecs 30000 --threads 8 \""
    Sargs=' -s3 0.35 -ec 0.65 '
    beta=`parse "1-0.$ialpha"`
    S3=`parse 0.35*$beta`
    EC=`parse 0.65*$beta`
    Mseq=" -s3 $S3 -ec $EC -sequence 0.$ialpha -alpha 0.$ialpha -wrappedArgs \"--maxsecs 44000 --threads 4 --custom ../../sequence/evalue/${G1}_${G2}.evalue auto distance clamp 0.$ialpha 0 -c 'pairWeightGED=$beta' -c 'pairWeightGraphlets=0'\""
    S3=`parse $S3/10`
    EC=`parse $EC/10`
    Sseq=" -ec $EC -s3 $S3 -sequence 0.9$ialpha -alpha 0.9$ialpha"
elif echo $M | grep -q -i '^gedevo$'; then
    Tcmp=m/9; Tmin=10; Tmax=30
    Margs=" -wrappedArgs \"--maxsecs 36000 --threads 8 \""
    Sargs=' -s3 0.35 -ec 0.65 '
    Mseq=" -wrappedArgs \"--maxsecs 3600 --threads 4 --blastpairlist sequence/bitscores/${G1}-${G2}.bitscores\""
    Sseq=" -ec .5 -s3 0.5 -sequence 0.5 "
elif echo $M | grep -q -i '^wave$'; then
    Margs=" -wrappedArgs sims/graphlet/$G1-$G2.sim "
    Sargs=' -s3 0 -wec 1 -wecnodesim graphlet '
    alpha=`parse .8+0.$ialpha/5`
    beta=`parse "1-$alpha"`
    Mseq=" -s3 $beta -sequence $alpha -wec $beta -alpha $alpha -wecnodesim graphlet -wrappedArgs sequence/graphlet+seq.$ialpha/${G1}-${G2}.sim"
    Sseq=" -s3 $beta -sequence $alpha -wec $beta -alpha $alpha -wecnodesim graphlet "
    Tcmp=m; Tmin=3; Tmax=20
elif echo $M | grep -q -i '^spinal$'; then
    Margs=""
    Sargs=''
    beta=`parse "1-0.$ialpha/10"`
    Mseq=" -sequence $beta -ec 0.$ialpha -alpha 0.$ialpha -wrappedArgs \"-I -ns sequence/SPINAL/${G1}-${G2}\""
    Sseq=" -ec 1 -sequence 300 "
    Tcmp=m; Tmin=3; Tmax=20
elif echo $M | grep -q -i '^ghost$'; then
    Tcmp=m/10; Tmin=10; Tmax=60
    Margs=""
    Sargs='-ec 0.65 -s3 0.35'
    beta=`parse "1-0.$ialpha"`
    Mseq="-alpha 0.$ialpha -s3 $beta -sequence 0.$ialpha -wrappedArgs '-alpha 0.$ialpha -sequencescores evalues/${G1}_${G2}.evalue'"
    Sseq="-ec 0.65 -s3 0.35 -sequence 0.$ialpha "
elif echo $M | grep -q -i '^sana+piswap$'; then
    M=sana+piswap
    $SEQ || die "sana+piswap needs sequence"
    beta=`parse "1-0.9$ialpha"`
    TL=`parse "($M1+$M2)/400"`
    # technically alpha should be 0.$ialpha but I want SANA to over-weight sequence by a factor of 10, so setting
    # -sequence $ialpha rather than 0.$ialpha accomplishes this: we'll actually get a normalized weighting of 0.9+alpha/10,
    # that is 0.91 when alpha=1 to 0.99 when alpha=9.
    # Note that the -alpha parameter is passed to PISWAP, but SANA will use 0.9+alpha/10 via the -sequence argument.
    Mseq=" -t $TL -tinitial auto -tdecay auto -sequence 0.9$ialpha -s3 $beta -alpha 0.$ialpha -wrappedArgs sequence/bitscore/${G1}_${G2}.bitscore "
    DIR=Mseq$ialpha
    set -x
    if $FORCE || [ ! -f $DIR/$M/$G1-$G2.out ]; then
	eval ./$M -method $M $Mseq -g1 $G1 -g2 $G2 -o $DIR/$M/$G1-$G2 >$DIR/$M/$G1-$G2.stdout 2>$DIR/$M/$G1-$G2.stderr
	rename 'SANA_PrePISWAP_;sana.' $DIR/$M/SANA_PrePISWAP_$G1-$G2*
    fi
    exit
elif echo $M | grep -q -i '^piswap$'; then
    Margs=" -wrappedArgs sims/graphlet/$G1-$G2.sim "
    Sargs=''
    Tcmp=m/10; Tmin=10; Tmax=60
else
    die "unknown method $M"
fi

set -x
if $SEQ; then
    DIR=Mseq$ialpha
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
