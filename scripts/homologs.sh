#!/bin/sh
USAGE="$0 [-c] [-1 species1 species2] column1 column2 [list of files]"
die() { echo "$USAGE" >&2; echo "$@" >&2; exit 1
}
one2one=0
s1=0
s2=0
COUNT=0
while :; do
    case $1 in
    -c) COUNT=1; shift;;
    -1) one2one=1
	s1=$2
	s2=$3
	shift 3;;
    *) break ;;
    esac
done
[ $# -ge 2 ] || die "wrong numebr of arguments"
col1=$1
col2=$2
shift 2

STDIN=""
if [ $# -eq 0 ]; then STDIN='-'; fi # if there are no arguments, then should read the standard input

awk 'BEGIN{o='$one2one';s1='$s1';s2='$s2';c1='$col1';c2='$col2'}
    ARGIND==1{geneHIDs[$3][$1][$2]++;HIDgenes[$1][$3][$2]++;HIDspec[$1][$2]++}
    ARGIND>1{if($c1 in geneHIDs)for(H in geneHIDs[$c1])if($c2 in HIDgenes[H] && $c2 in geneHIDs && H in geneHIDs[$c2])
	{
	    if(!o){
		printf "%s\t%s",$c1,$c2
		if('$COUNT')for(S1 in geneHIDs[$c1][H])for(S2 in geneHIDs[$c2][H])
		    printf "\t[(%s %d),(%s %d)]",S1,HIDspec[H][S1],S2,HIDspec[H][S2]
	    }
	    else if(s1 in geneHIDs[$c1][H] && s2 in geneHIDs[$c2][H] && HIDspec[H][s1]==1 && HIDspec[H][s2]==1)
	    {
		printf "%s\t%s",$c1,$c2
		if('$COUNT')for(S1 in geneHIDs[$c1][H])for(S2 in geneHIDs[$c2][H])
		    printf "\t[(%s %d),(%s %d)]",S1,HIDspec[H][S1],S2,HIDspec[H][S2]
	    }
	    else next;
	    print ""
	}
    }' /home/sana/sequence/homologene/homologene.tsv $STDIN "$@"
