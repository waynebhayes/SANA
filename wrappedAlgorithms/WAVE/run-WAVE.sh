#!/bin/sh
die(){ echo "FATAL ERROR: $@" >&2; exit 1
}
USAGE="$0 g1.gw g2.gw simsFile outputFile
where these all use names for protein; WAVE expects all ints, to we convert to WAVE format and back."

[ -f "$1" ] || die "first arg: graph1 in GW format"
[ -f "$2" ] || die "second arg: graph2 in GW format"
[ -f "$3" ] || die "third arg: sims file in 3-column format: nodeFromG1 nodeFromG2 sim"
[ -f "$4" ] && die "output file already exists!"

trap "/bin/rm -f '$4.sims' " 0 1 2 3 15

# Convert LEDA input files to WAVE numerical-only edge-list
N=0
echo -n > "$4.NameId.txt"
for i in "$1" "$2"; do
    N=`expr $N + 1`
    b=`basename "$i" .gw`
    cat "$i" | awk '
        /\|{/&&NF==1{gsub("[|{}]","",$1); num[$1]=n; printf "NameIdMap G'$N' %s %d\n", $1, n >> "'$4'.NameId.txt"; n++}
        NF==4{edge[m++]=sprintf("%d %d", $1-1,$2-1)}
        END{printf "%d %d\n",n,m; for(i=0;i<m;i++)printf "%s\n", edge[i]}' > $b.el
done

#create numerical-only sims file
cat "$4.NameId.txt" "$3" | awk 'NF==4&&/NameIdMap G1/{num1[$3]=$4}
        NF==4&&/NameIdMap G2/{num2[$3]=$4}
        NF==3{print num1[$1], num2[$2], $3}' > "$4.sims"

./WAVE "`basename $1 .gw`.el" "`basename $2 .gw`.el" "$4.sims" "$4"

#Translate back into names to return named alignment.
cat "$4.NameId.txt" "$4" | awk 'NF==4&&/NameIdMap G1/{name[$4]=$3}
        NF==4&&/NameIdMap G2/{name[$4]=$3}
        NF==2{print name[$1], name[$2]}' > "$4.align"
