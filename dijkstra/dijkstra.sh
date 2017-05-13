#!/bin/sh
die(){ echo "FATAL: $@" >&2; exit 1
}
USAGE="$0 graph1.gw graph2.gw simFile Delta"
TMPDIR=/tmp/dijktra$$
#trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir $TMPDIR

[ $# -eq 4 ] || die "$USAGE"
G1="$1"
G2="$2"
simFile="$3"
delta="$4"

# Create the node and edge files, numbered from zero.
awk 'BEGIN{N=0}/\|{/{if(NF==1){gsub("[|{}]",""); print N, $0 > "'"$TMPDIR/"'nodes1.txt"; N++}else{print $1-1,$2-1 >"'"$TMPDIR/"'edges1.txt"}}' $G1
awk 'BEGIN{N=0}/\|{/{if(NF==1){gsub("[|{}]",""); print N, $0 > "'"$TMPDIR/"'nodes2.txt"; N++}else{print $1-1,$2-1 >"'"$TMPDIR/"'edges2.txt"}}' $G2

#Now map the simfile to integers: note that the simfile has columns reversed!
#Note that internally I need to map to 1 so that the boolean existence check works, but back to 0 on output.
(cat $TMPDIR/nodes1.txt; echo G2; cat $TMPDIR/nodes2.txt; echo simFile; wzcat $simFile) | 
    awk '/^simFile$/{simFile=1;next}/^G2$/{G2=1;next}{if(simFile){if(!G1int[$1]||!G2int[$2]) {print "ERROR: cannot find one of the nodes " $1 " or " $2 >"/dev/fd/2"; exit 1}; print G2int[$2]-1,G1int[$1]-1,$3 > "'"$TMPDIR/"'simFile"} else if(G2)G2int[$2]=$1+1;else G1int[$2]=$1+1}'
