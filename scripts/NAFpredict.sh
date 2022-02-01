#!/bin/bash
USAGE="$0 [-a] [-GO1freq k] gene2goWhich oldG1.el oldG2.el seqSim GO1 GO2 tax1 tax2 NAFthresh col1 col2 outName [files]
NOTE: predictions go to outName-p, validations to outName-v, summary to stdout
where:
seqSim is the file with sequence similarities/orthologs used to eliminate predictions possible to predict by sequence
GO1 is the name of source gene2go file used to predict annotations in GO2 (NOT including allGO or NOSEQ part of filename)
tax1 and tax2 are the predictor and predictee taxonomic IDs (eg 10090 for mouse, 9606 for human)
NAFthresh is the lower predictive bound, and has a leading dash if you want to predict in the opposite direction
    to the directory names (eg 0 to predict MM->HS or -0 for MM<-HS)
col1,col2 are the columns in the files where protein names are found
files contain at least 2 columns with aligned protein pairs, including duplicates and NO count
    (we will compute NAF in the script) [no files means you're sending to stdin]
gene2goWhich: should be NOSEQ, allGO, or any other extension that exists

Options:
    -a : allow all predictions, even those not validatable (NOT recommended! default is to restrict to validatable)
    -GO1freq k: remove predictions of GO terms with frequency above k"

die(){ echo "$USAGE">&2; echo "$@" >&2; exit 1
}
# Get rid of annoying NOT lines...
notNOT() { grep -v '	NOT	' "$@"
}

TMPDIR=/tmp/GOpredict.$$
 trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 15
 trap "trap '' 0 1 2 15; echo TMPDIR is $TMPDIR >&2; exit 1" 3
mkdir $TMPDIR || die "Hmmm, $TMPDIR could not make $TMPDIR"

ALLOW_ALL=false
GO1freq=2000000000 # big enough for default...
while true; do
    case "$1" in
    -a) ALLOW_ALL=true; shift;;
    -GO1freq) GO1freq="$2"; shift 2;;
    -*) die "unknown option '$1'";;
    *) break;;
    esac
done

[ $# -ge 12 ] || die "expecting at least 12 args, not $#"
GENE2GO="$1"
G1="$2"
G2="$3"
seqSim="$4"
GO1="$5"
GO2="$6"
tax1=$7
tax2=$8
NAFthresh=$9
c1=${10}
c2=${11}
outName=${12}
shift 12

# Evidence codes: all, NOSEQ, and SEQ
EVC_NOS="EXP HDA HEP HGI HMP IC IDA IEP IGI IKR IMP IMR IPI IRD NAS ND TAS"
EVC_SEQ="IBA IEA IGC ISA ISM ISO ISS RCA"
EVC_ALL="$EVC_NOS $EVC_SEQ"

[ -f $G1 -a -f $G2 ] || die "need network files $G1 and $G2"
[ -f $outName-v -o -f $outName-p ] && die "refusing to overwrite existing $outName-[pv]"
for g in allGO NOSEQ; do
    [ -f "$GO1.$g" ] || die "can't find gene2go file '$GO1.$g'"
    [ -f "$GO2.$g" ] || die "can't find gene2go file '$GO2.$g'"
done

echo $EVC_SEQ | newlines | awk '{printf "\t%s\t\n",$0}' > $TMPDIR/EVC_SEQ # sequence evidence codes
cat "$seqSim" > $TMPDIR/seqSim # in case it's a named pipe, we need to store it

# Annotations that are already known for the target species at time t1:
grep "^$tax2	" $GO1.allGO | cut -f1-3 | sort -u >$TMPDIR/GO1.tax2.allGO.1-3

# All annotations at time t2 (the validatable set), with their validating evidence codes (nothing to do with the evidence
# codes in the PREDICTING set).
grep "^$tax2	" $GO2.$GENE2GO | cut -f1-4 | sort -u | tee $TMPDIR/GO2.tax2.$GENE2GO.1-4 |
    fgrep -f $TMPDIR/EVC_SEQ | # extract annotations with sequence-based evidence
    cut -f1-3 | sort -u >$TMPDIR/GO2.tax2.SEQ.1-3 # sequence-based annotations discovered by the time of GO2

PGO2="$GO2" # Argument representing GO2 for Predictable.sh script
if $ALLOW_ALL; then
    PGO2=NONE # Predictable.sh will not restrict based on validatable annotations
fi
./Predictable.sh -GO1freq $GO1freq -gene2go $GENE2GO $tax1 $tax2 $GO1 $PGO2 $G2 | # Predictable-in-principle, with SOURCE evidence codes
    tee $TMPDIR/Predictable.Vable | # validatable=(predictable-in-principle annotations) \INTERSECT (actual annotations in GO2)
    fgrep -v -f $TMPDIR/GO1.tax2.allGO.1-3 | # remove predictions already known in target species at earlier date
    fgrep -v -f $TMPDIR/GO2.tax2.SEQ.1-3 | # remove sequence-based annotations discovered by later date
    tee $TMPDIR/Predictable.Vable.notGO1.notSEQ2 | # list of interest, with PREDICTING evidence code
    cut -f1-3 | sort -u > $TMPDIR/Predictable.Vable.notGO1.notSEQ2.1-3 # final set (RECALL denom), without evidence codes
#grep '	NOT	' $TMPDIR/* && die "NOT fields found after Predictable.sh was run"

dataDir=`echo "$@" | newlines | sed 's,/[^/]*$,,' | sort -u`
sort "$@" | uniq -c | sort -nr | awk 'BEGIN{tax1='$tax1';tax2='$tax2';
	c1=1+'$c1';c2=1+'$c2'; # increment column since "uniq -c" above prepends NAF to the line
	NAFthresh='$NAFthresh';
    }
    ARGIND==1{seq[$1][$2]=1;next} # orthologous & sequence similar pairs
    ARGIND==2{u=$c1;;v=$c2;
	if(u in seq && v in seq[u])next; # ignore known orthology or sequence similarity
	NAF[u][v]=$1; # store ALL NAFs for now, not just those above the threshold, because
	    # later we allow the total score of v to be additive across multiple nodes u.
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
				# Note that this will bump the NAF of p2 for multiple p1s it is aligned to--by construction
				NAFpredict[tax2][p2][g][evc]+=NAF[p1][p2]
				NAFpredict[tax2][p2][g]["ALL"]+=NAF[p1][p2] # "ALL" accounts for all evidence codes.
			    }
	# Now that we have accumulated all the possible predictions and evidences, print out only those that meet NAFthresh
	for(p2 in NAFpredict[tax2])for(g in NAFpredict[tax2][p2])for(evc in NAFpredict[tax2][p2][g])
	    if(NAFpredict[tax2][p2][g][evc] >= NAFthresh) {
		# it is a prediction!!! Print out the NAF and the expected line to find in the later gene2go file:
		printf "%d\t%d\t%s\t%s\t%s\t%s\n",NAFpredict[tax2][p2][g][evc],tax2,p2,g,evc,C[g]
		# Note, however, that when evc="ALL", grepping for the above line will not match any lines in a gene2go file.
	    }
    }' "$TMPDIR/seqSim" - "$GO1.$GENE2GO" |
	fgrep -v -f $TMPDIR/GO1.tax2.allGO.1-3 | # remove ones that are already known (with any evidence) at the earlier date
	fgrep -v -f $TMPDIR/GO2.tax2.SEQ.1-3 |   # remove ones discovered using sequence evidence at later date
	sort -nr | # sort highest first;do NOT remove duplicates: same p2 predicted from diff p1s (or diff evcs) has meaning
	tee $TMPDIR/predictions.NAF,1-4,8 |
	cut -f2-4 | # remove NAF, evCode & Cat from prediction before search (evc is PREDICTING evidence code!)
	sort -u | fgrep -f - "$GO2.$GENE2GO" | # if we validate with allGO we could worry about removing SEQ later.
	sort -u > $TMPDIR/validated.$GENE2GO
#grep '	NOT	' $TMPDIR/* && die "NOT fields found after predictions made"

# Now process
evCodesPred=`cut -f5 $TMPDIR/predictions.NAF,1-4,8 | grep -v ALL | sort -u` # col5 because of leading NAF
allRegExp="(`echo $evCodesPred | sed 's/ /|/g'`)"
for evc in $evCodesPred $allRegExp; do
    PIP=`egrep "	$evc	" $TMPDIR/Predictable.Vable.notGO1.notSEQ2 | cut -f1-3 | sort -u | wc -l`
    pred=`egrep "	$evc	" $TMPDIR/predictions.NAF,1-4,8 | cut -f2-4 | sort -u | tee $TMPDIR/pred$evc | wc -l`
    val=`fgrep -f $TMPDIR/pred$evc $TMPDIR/Predictable.Vable.notGO1.notSEQ2.1-3 | sort|tee $TMPDIR/V1 | fgrep -f - $TMPDIR/validated.$GENE2GO | cut -f1-3 | sort -u | tee $TMPDIR/val$evc | wc -l`
    #DF_FILES="$TMPDIR/V1 $TMPDIR/val$evc"; diff $DF_FILES >/dev/tty || die "oops, $DF_FILES differ"
    echo "$evc $pred $PIP $val" |
	awk '$2>0{printf "%10s %3s NAF %3d : %6d pred %6d PIP %5d val prec %5.1f%%\n",
	    "'$dataDir'",$1,'$NAFthresh',$2,$3,$4,100*$4/$2}'
done > $TMPDIR/predECSummary
Categories=`cut -f8 $TMPDIR/validated.$GENE2GO | sort -u`
for c in $Categories; do
    PIP=`grep "	$c" $TMPDIR/Predictable.Vable.notGO1.notSEQ2 | cut -f1-3 | sort -u | wc -l`
    pred=`grep "	$c" $TMPDIR/predictions.NAF,1-4,8 | cut -f2-4 | sort -u | tee $TMPDIR/pred$c | wc -l`
    val=`fgrep -f $TMPDIR/pred$c $TMPDIR/Predictable.Vable.notGO1.notSEQ2.1-3 | sort|tee $TMPDIR/V1 | fgrep -f - $TMPDIR/validated.$GENE2GO | cut -f1-3 | sort -u | tee $TMPDIR/val$c | wc -l`
    #DF_FILES="$TMPDIR/V1 $TMPDIR/val$c"; diff $DF_FILES >/dev/tty || die "oops, $DF_FILES differ"
    echo "$c $pred $PIP $val" |
	awk '$2>0{printf "%10s %9s NAF %3d : %6d pred %6d PIP %5d val prec %5.1f%%\n",
	    "'$dataDir'",$1,'$NAFthresh',$2,$3,$4,100*$4/$2}'
done > $TMPDIR/predCatSummary
#grep '	NOT	' $TMPDIR/* && die "NOT fields found after validation step"
# Shorten GO fileNames for output
GO1=`echo $GO1|sed 's,^.*/go/,,'`
GO2=`echo $GO2|sed 's,^.*/go/,,'`
echo "Predictions by evidence code for $dataDir $GO1 -> $GO2, NAF $NAFthresh"
lastCol=`awk '{print NF}' $TMPDIR/predECSummary | sort | uniq -c | sort -nr | head -1 | awk '{print $2}'`
[ "X$lastCol" != X ] && sort -k ${lastCol}gr $TMPDIR/predECSummary
echo "Predictions by GO hierarchy for $dataDir $GO1 -> $GO2, NAF $NAFthresh"
lastCol=`awk '{print NF}' $TMPDIR/predCatSummary | sort | uniq -c | sort -nr | head -1 | awk '{print $2}'`
[ "X$lastCol" != X ] && sort -k ${lastCol}gr $TMPDIR/predCatSummary

mv $TMPDIR/predictions.NAF,1-4,8 $outName-p # includes leading NAF+duplicates, each should be from a different p1 in tax1
mv $TMPDIR/validated.$GENE2GO $outName-v
[ $NAFthresh -eq 1 ] && mv $TMPDIR/Predictable.Vable.notGO1.notSEQ2 $outName-PIP # same for all NAF values so only copy for NAF 1
