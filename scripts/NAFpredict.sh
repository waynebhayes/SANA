#!/bin/bash
USAGE="$0 net1.el net2.el seqSim go1 go2 tax1 tax2 NAFthresh col1 col2 outName [Resnik.files]
NOTE: predictions go to outName-p, validations to outName-v, summary to stdout
where:
seqSim is the file with sequence similarities/orthologs used to eliminate predictions possible to predict by sequence
go1 is the basename of source gene2go file which makes predictions which are searched in go2
    (do not include the allGO or NOSEQ part, that is done automatically)
tax1 and tax2 are the predictor and predictee taxonomic IDs (eg 10090 for mouse, 9606 for human)
NAFthresh is the lower predictive bound, and has a leading dash if you want to predict in the opposite direction
    to the directory names (eg 0 to predict MM->HS or -0 for MM<-HS)
col1,col2 are the columns in the files where protein names are found
files contain at laest 2 columns with aligned protein pairs, including duplicates and NO count
    (we will compute NAF in the script) [no files means you're sending to stdin]
-p means list all predictions (turns off counting)
-v means list the validated  predictions (turns off counting)
"

die(){ echo "$USAGE">&2; echo "$@" >&2; exit 1
}

# Evidence codes: all, NOSEQ, and SEQ
EVC_ALL="EXP HDA HEP HGI HMP IBA IC IDA IEA IEP IGC IGI IKR IMP IMR IPI IRD ISA ISM ISO ISS NAS ND RCA TAS"
EVC_NOS="EXP HDA HEP HGI HMP IC IDA IEP IGI IKR IMP IMR IPI IRD NAS ND TAS"
EVC_SEQ="IBA IEA IGC ISA ISM ISO ISS RCA"

case "$1" in
-*) die "unknown option '$1'";;
esac

[ $# -ge 11 ] || die "expecting at least 11 args, not $#"
G1="$1"
G2="$2"
seqSim="$3"
go1="$4"
go2="$5"
tax1=$6
tax2=$7
NAFthresh=$8
c1=$9
c2=${10}
outName=${11}
shift 11
[ -f $G1 -a -f $G2 ] || die "need network files $G1 and $G2"
[ -f $outName-v -o -f $outName-p ] && die "refusing to overwrite existing $outName-[pv]"
for g in allGO NOSEQ; do
    [ -f "$go1.$g" ] || die "can't find gene2go file '$go1.$g'"
    [ -f "$go2.$g" ] || die "can't find gene2go file '$go2.$g'"
done

TMPDIR=/tmp/GOpredict.$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir $TMPDIR
echo $EVC_SEQ | newlines | awk '{printf "\t%s\t\n",$0}' > $TMPDIR/EVC_SEQ # sequence evidence codes
cat "$seqSim" > $TMPDIR/seqSim # in case it's a named pipe, we need to store it

# "Predictions" that are already known for the target species at time t1:
grep "^$tax2	" $go1.allGO | cut -f1-3 | sort -u >$TMPDIR/go1.tax2.already
# "Predictions" that had been derived via sequence for the target species by time t2:
grep "^$tax2	" $go2.allGO | fgrep -f $TMPDIR/EVC_SEQ | cut -f1-3 | sort -u >$TMPDIR/go2.tax2.already
#Predictable-in-principle
./Predictable.sh $tax1 $tax2 $go1 $go2 $G2 > $TMPDIR/Predictable

dataDir=`echo "$@" | newlines | sed 's,/[^/]*$,,' | sort -u`
sort "$@" | uniq -c | sort -nr | awk 'BEGIN{tax1='$tax1';tax2='$tax2';
	c1=1+'$c1';c2=1+'$c2'; # increment column since "uniq -c" above prepends NAF to the line
	NAFthresh='$NAFthresh';
    }
    ARGIND==1{seq[$1][$2]=1;next} # sequence similar pairs
    ARGIND==2{u=$c1;;v=$c2;
	if(u in seq && v in seq[u])next; # ignore known orthology or sequence similarity
	NAF[u][v]=$1; # store ALL NAFs for now, not just those above the threshold
	next
    }
    ARGIND==3{FS="	"; if(/	NOT	/)next} # make FS a tab, and ignore "NOT" lines
    ARGIND==3&&($1==tax1||$1==tax2){++pGO[$1][$2][$3][$4]; # species, protein, GO, evidence code
	C[$3]=$NF; # Category (BP,MF,CC)
    }
    END{
	for(p1 in pGO[tax1]) # loop over all proteins in species1 that have any GO terms
	    if(p1 in NAF) # if we have a predictive NAF value for that protein...
		for(p2 in NAF[p1]) # loop over all the species2 proteins aligned to p1
		    for(g in pGO[tax1][p1]) # loop over all the GO terms from protein p1
			# if protein p2 is not listed at all in the gene2go file...
			#... or if it is but does not have this particular GO term...
			if(!(p2 in pGO[tax2]) ||!(g in pGO[tax2][p2]))
			    for(evc in pGO[tax1][p1][g]) {
				NAFpredict[tax2][p2][g][evc]+=NAF[p1][p2]
				NAFpredict[tax2][p2][g]["ALL"]+=NAF[p1][p2]
			    }
	# Now that we have accumulated all the possible predictions and evidences, print out only those that meet NAFthresh
	for(p2 in NAFpredict[tax2])for(g in NAFpredict[tax2][p2])for(evc in NAFpredict[tax2][p2][g])
	    if(NAFpredict[tax2][p2][g][evc] >= NAFthresh) { # do we want EACH evidence above NAFthresh? ALL accounts this.
		# it is a prediction!!! Print out the NAF and the expected line to find in the later gene2go file:
		printf "%d\t%d\t%s\t%s\t%s\t%s\n",NAFpredict[tax2][p2][g][evc],tax2,p2,g,evc,C[g]
	    }
    }' "$TMPDIR/seqSim" - "$go1.NOSEQ" | # make predictions using ????
	fgrep -v -f $TMPDIR/go1.tax2.already | # remove ones that are already known in allGO
	sort -nr | # sort highest first but do NOT remove duplicates: the same p2 predicted from different p1s means something
	tee $TMPDIR/predictions.allCol |
	cut -f2-4 | # remove NAF, evidence code &hier from prediction before search
	sort -u | fgrep -f - "$go2.NOSEQ" | # validate in NOSEQ
	fgrep -v '	NOT	' | fgrep -v -f $TMPDIR/go2.tax2.already | # remove seq-based discovered by t2
	sort -u > $TMPDIR/validated.allCol

# Now process
evCodesPred=`cut -f5 $TMPDIR/predictions.allCol | sort -u` # col5 because of leading NAF
evCodesVal=`cut -f4 $TMPDIR/validated.allCol | sort -u`
Categories=`cut -f8 $TMPDIR/validated.allCol | sort -u`
# Shorten GO fileNames for output
GO1=`echo $go1|sed 's,^.*/go/,,'`
GO2=`echo $go2|sed 's,^.*/go/,,'`
for evc in $evCodesPred; do
    pred=`fgrep "	$evc	" $TMPDIR/predictions.allCol | cut -f2-4 | sort -u | tee $TMPDIR/pred$evc | wc -l`
    PIP=`cut -f1-3 $TMPDIR/Predictable | sort -u | fgrep -f - $go2.NOSEQ | fgrep -c "	$evc	"`
    val=`cut -f1-3 $TMPDIR/Predictable | fgrep -f - $TMPDIR/pred$evc | cut -f1-3 | fgrep -f - $TMPDIR/validated.allCol | cut -f1-3 | sort -u | tee $TMPDIR/val$evc | wc -l`
    echo "$evc $pred $PIP $val" |
	awk '$2>0{printf "%10s %3s NAF %3d : %6d pred %6d PIP %5d val prec %5.1f%%\n",
	    "'$dataDir'",$1,'$NAFthresh',$2,$3,$4,100*$4/$2}'
done > $TMPDIR/predECSummary
for c in $Categories; do
    pred=`fgrep "	$c" $TMPDIR/predictions.allCol | cut -f2-4 | sort -u | tee $TMPDIR/pred$c | wc -l`
    PIP=`cut -f1-3 $TMPDIR/Predictable | sort -u | fgrep -f - $go2.NOSEQ | fgrep -c "	$c"`
    val=`fgrep -f $TMPDIR/pred$c $TMPDIR/validated.allCol | cut -f1-3 | sort -u | tee $TMPDIR/val$c | wc -l`
    echo "$c $pred $PIP $val" |
	awk '$2>0{printf "%10s %9s NAF %3d : %6d pred %6d PIP %5d val prec %5.1f%%\n",
	    "'$dataDir'",$1,'$NAFthresh',$2,$3,$4,100*$4/$2}'
done > $TMPDIR/predCatSummary
echo "Predictions by evidence code for $dataDir $GO1 -> $GO2, NAF $NAFthresh"
lastCol=`awk '{print NF}' $TMPDIR/predECSummary | sort | uniq -c | sort -nr | head -1 | awk '{print $2}'`
[ "X$lastCol" != X ] && sort -k ${lastCol}gr $TMPDIR/predECSummary
echo "Predictions by GO hierarchy for $dataDir $GO1 -> $GO2, NAF $NAFthresh"
lastCol=`awk '{print NF}' $TMPDIR/predCatSummary | sort | uniq -c | sort -nr | head -1 | awk '{print $2}'`
[ "X$lastCol" != X ] && sort -k ${lastCol}gr $TMPDIR/predCatSummary

mv $TMPDIR/predictions.allCol $outName-p # includes leading NAF+duplicates, each should be from a different p1 in tax1
mv $TMPDIR/Predictable $outName-PIP
mv $TMPDIR/validated.allCol $outName-v
