#!/bin/bash
USAGE="USAGE: $0 [-already1] [-already2] tax1 tax2 GO1 GO2 net2.old
    where GO1 and GO2 do NOT include the allGO or NOSEQ extension
Purpose: produce the list of 'predictable-in-principle' tax2-protein-GO triplets, eliminating any triplets with sequence-based
    evidence at EITHER the earlier or later time, no matter whether they also have non-sequence-based evidence.
    Arguments: tax1 is the source species for the
    predictions, tax2 is the target species, GO1 and GO2 are the paths to the gene2go files for the old and new times,
    net2.old is the old version of the target species' PPI network."
die() { (echo "$USAGE"; echo "FATAL ERROR: $@") >&2; exit 1
}

ALREADY1=false
ALREADY2=false
while true; do
    case "$1" in
    -a*1) ALREADY1=true;;
    -a*2) ALREADY2=true;;
    -*) die "unknown option '$1'";;
    *) break;;
    esac
done

[ $# -eq 5 ] || die "expecting exactly 6 arguments"
tax1=$1;tax2=$2
GO1=$3; GO2=$4
G2old=$5
SEQ="(IBA|IEA|IGC|ISA|ISM|ISO|ISS|RCA)" # egrep expression matching all sequence-based evidence codes

TMPDIR=/tmp/PIP.$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir -p $TMPDIR

# This script returns all species-protein-GO triplets for species $1 from gene2go file $2.allGO that used any sequence evidence
getSeqSuppTriplets() {
    grep "^$1	" $2.allGO | # extract *ALL* annotations from our target species
    egrep "	($SEQ)	" | # extract those that use sequence evidence
    cut -f1-3
}
# Opposite of the above: list of annotation lines for species $1 from gene2go files $2 that had no sequence evidence codes
getSeqFreeTriplets() {
    getSeqSuppTriplets $1 $2 | # extract sequence-supported annotations for species 1 from gene2go file $2
    fgrep -v -f - $2.NOSEQ | # remove sequence-supported annotations from NOSEQ, even those with non-sequence evidence
    grep "^$1	" | # extract our target species, returning only those annotations that had no sequence evidence
    cut -f1-3
}

if $ALREADY1; then
    # ALL "already-known" tax2-protein-GO triplets for target species at the earlier time (ANY evidence code)
    grep "^$tax2	" $GO1.allGO | cut -f1-3 > $TMPDIR/GO1.tax2.already
fi
if $ALREADY2; then
    # ALL "already-known" tax2-protein-GO triplets for target species at the later time (ANY evidence code)
    grep "^$tax2	" $GO2.allGO | cut -f1-3 > $TMPDIR/GO2.tax2.already
fi

# The ultimate source of our predictions: sequence-free triplets from the source species at the earlier time
getSeqFreeTriplets $tax1 $GO1 > $TMPDIR/GO1.tax1.seq-free

# The most basic set of Predictable-in-Principle 
getSeqFreeTriplets $tax2 $GO2 | # all seq-free annotations at the later date---ie., the target list we're trying to predict
    fgrep -v -f $TMPDIR/GO1.tax2.already | # remove any that were already known via ANY evidence at the earlier date
    awk 'ARGIND==1{++deg[$1];++deg[$2];next} # get list of nodes in target species, earlier network
	ARGIND==2&&$1=='$tax1'{++GOtax1[$3][$4]} # get list of all earlier GO terms and their evidence from source species
	ARGIND==3&&$2 in deg&&$3 in GOtax1{
	    # predictable annotation if node is in earlier target network and earlier source network has such a GO term
	    for(e in GOtax1[$3]) printf "%s\t%s\t%s\t%s\t%s\n",$1,$2,$3,e,$NF
	}' $G2old $TMPDIR/GO1.tax1.seq-free - |
    fgrep -v -f <(
	    grep "^$tax2	" $GO1.allGO | cut -f1-3 # already known at earlier time
	    grep "^$tax2	" $GO2.allGO | fgrep -f $SEQ | cut -f1-3 # discovered using sequence at later time
	) | sort -u
# note the evidence code listed here is the PREDICTING (ie., old) evidence code, NOT the one you'll find in the new GO file
