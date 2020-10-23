#!/bin/sh
USAGE="MNC=Multi-Node-Correctness <multi-dir>"
die() { echo "$@" >&2; exit 1
}
[ -d "$1" ] || die "first arg must be directory name of the run"
TMP="$1/MNC"
trap "/bin/rm -rf $TMP; exit" 0 1 2 3 15
mkdir -p $TMP
cd $1
cat *s/??/multiAlign.tsv | sort -S1G | uniq -c | awk '{delete K; for(i=2;i<=NF;i++)++K[$i];delete K["_"];biggest=0;for(i in K)if(K[i]>biggest)biggest=K[i];print $1,biggest}' > "$TMP/bigs" & # get this running so pearson will be fast later

numNets=`ls | head -1 | awk '{printf "cat %s/01/multiAlign.tsv\n",$1}' | sh | awk '{print NF}' | uniq -c | sort -n | tail -1 | awk '{print $2}'`
echo -n "step	clusters	empty	nomatch"; for i in `integers 2 $numNets`; do echo -n "	NC$i"; done
echo "	weighted	unweighted"
for d in *s/??
do
    [ -f "$d/multiAlign.tsv" ] || continue
    echo -n "$d	" | sed -e 's,00s/,,' -e 's,0s/,,'
    awk '{
	# Here you only get points for one of the largest, if there are multiple
	delete K
	for(i=1;i<=NF;i++){node=$i; ++K[node]}
	biggest=0;
	for(node in K) {
	    if(node=="_") NC[0] += K[node]; # empty locations
	    else {
		++NC[K[node]];
		if(K[node]>biggest) biggest=K[node];
	    }
	}
	columns=NF-K["_"]
	if(biggest>1) {
	    sumNC+=biggest/columns
	    wSumNC+=biggest
	}
	weight+=columns

	# Here you only get points for every node pair on the line that is the same.
	for(i=1;i<NF;i++)for(j=i+1;j<=NF;j++)if($i!="_" && $j!="_"){possible++;if($i==$j)correct++}
    }
    END{
	printf "%8d",NR
	for(i=NF;i>1;i--)NC[i-1]+=NC[i];
	for(i=0;i<=NF;i++)printf "\t%d", NC[i]
	printf "\t%.6f\t%.6f\n",wSumNC/weight,correct/possible
    }' "$d/multiAlign.tsv"
done

echo ""
echo "Pearson of core frequency-vs-biggest: "
wait
pearson -V < $TMP/bigs

