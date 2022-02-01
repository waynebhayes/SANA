#/bin/sh
################## SKELETON: DO NOT TOUCH THESE 2 LINES
BASENAME=`basename "$0" .sh`; TAB='	'; NL='
'
#################### ADD YOUR USAGE MESSAGE HERE, and the rest of your code after END OF SKELETON ##################
USAGE="USAGE: $BASENAME net1.el net2.el {list of 100 alignment files for computing NAF}
PURPOSE: given 100 alignments, spit out the netbits support for each NAF value"

################## SKELETON: DO NOT TOUCH CODE HERE
# check that you really did add a usage message above
USAGE=${USAGE:?"$0 should have a USAGE message before sourcing skel.sh"}
die(){ echo "$USAGE${NL}FATAL ERROR in $BASENAME:" "$@" >&2; exit 1; }
[ "$BASENAME" == skel ] && die "$0 is a skeleton Bourne Shell script; your scripts should source it, not run it"
echo "$BASENAME" | grep "[ $TAB]" && die "Shell script names really REALLY shouldn't contain spaces or tabs"
[ $BASENAME == "$BASENAME" ] || die "something weird with filename in '$BASENAME'"
warn(){ (echo "WARNING: $@")>&2; }
not(){ if eval "$@"; then return 1; else return 0; fi; }
newlines(){ awk '{for(i=1; i<=NF;i++)print $i}' "$@"; }
parse(){ awk "BEGIN{print $*}" </dev/null; }

# Temporary Filename + Directory (both, you can use either, note they'll have different random stuff in the XXXXXX part)
TMPDIR=`mktemp -d /tmp/$BASENAME.XXXXXX`
 trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15 # call trap "" N to remove the trap for signal N

#################### END OF SKELETON, ADD YOUR CODE BELOW THIS LINE

net1=$1
net2=$2
shift 2

[ $# -eq 100 ] || die "expecting 100 alignment files"
[ `ls -d "$@" | fgrep -c .align` -eq 100 ] || die "expecting 100 alignment files"

for i; do
    case "$i" in
    *.align) b=`basename $i .align`;;
    *) die "'$i' is not an alignment file";;
    esac
    ccs-el.sh "$net1" "$net2" $i | tee $b.ccs-el | awk '{printf "\t%s\t\n",$0}' > $b.ccs-el.tt
    [ -f $b.resnik.NOSEQ ] || die "expecting $b.resnik.NOSEQ to exist"
done

( echo 'NAF	u	v	Resnik'; sort [A-Z][A-Z][A-Z][A-Z][0-9]*_60.resnik.NOSEQ | uniq -c | sort -nr | sed 's/max	//' -e 's/^  *//' -e 's/ /	/g') > NAF-uv-resnik.NOSEQ.tsv

NAFs=`awkcel '{print NAF}' NAF-uv-resnik.NOSEQ.tsv | uniq`

for N in $NAFs; do
    awk 'NR>1&&$1>='$N'{printf "%s,%s\n",$2,$3}' NAF-uv-resnik.NOSEQ.tsv | # get the list of aligned pairs above NAF threshold
	tee /tmp/NAF.$N.nodes |
	awk '{printf "\t%s\t\n",$0}' | # add leading and trailing tabs to isolate them for fgrep...
	fgrep -l -f - *.ccs-el.tt | sed 's/\.tt$//' | # get rid of the .tt extension to cat only the ccs-el files...
	xargs cat > /tmp/NAF.$N.el # use "cat", not "sort -u", to count edges, including dups, that support a NAF-aligned pair
    count_el=`induce /tmp/NAF.$N.nodes /tmp/NAF.$N.el | # get the NAF-induced subgraph on the multi-edged network
	count.el -`
    echo $N $count_el |
	hawk '{NAF=$1; n=$2;m=$3;b=netbits(n); if(m) print NAF,n,m,b,m/b}' # print NAF, nodes, edges, netbits, edges/netbits
done | revlines | # reverse the lines so we see the LOWEST NAF values first...
    awk 'BEGIN{print "NAF	nodes	edges	netbits	ratio	avgRatio"}
	{sum+=$NF; print $0,sum/NR}' | # and then print the above lines with the AVERAGE support up to this NAF
	sed 's/^  *//' -e 's/ *$//' -e 's/  */	/g' # make it a TSV
