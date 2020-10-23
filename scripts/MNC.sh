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

echo "step	clusters	weighted	unweighted"
for d in *s/??
do
    [ -f "$d/multiAlign.tsv" ] || continue
    echo -n "$d	" | sed -e 's,00s/,,' -e 's,0s/,,'
    awk '{
	# Here you only get points for one of the largest, if there are multiple
	delete K
	for(i=1;i<=NF;i++)++K[$i]
	biggest=0; # note "_" is an empty column
	for(i in K)if(i!="_" && K[i]>biggest)biggest=K[i]
	columns=NF-K["_"]
	if(biggest>1) {
	    sumNC+=biggest/columns
	    wSumNC+=biggest
	}
	weight+=columns

	# Here you only get points for every node pair on the line that is the same.
	for(i=1;i<NF;i++)for(j=i+1;j<=NF;j++)if($i!="_" && $j!="_"){possible++;if($i==$j)correct++}
    }
    END{printf "%8d\t%.6f\t%.6f\n",NR,wSumNC/weight,correct/possible}' "$d/multiAlign.tsv"
done

echo ""
echo Last step:
echo "matches	count	score"
lastAlig=`ls *s/??/multiAlign.tsv | tail -1`
numNets=`awk '{print NF}' $lastAlig | uniq -c | sort -n | tail -1 | awk '{print $2}'`
for k in `integers 2 $numNets`; do gawk '{delete K;for(i=1;i<=NF;i++)++K[$i];for(i in K)if(K[i]>='$k')nc++}END{printf "%7d\t%5d\t%.6f\n",'$k',nc,nc/NR}' $lastAlig
done | tee $TMP/MNC.txt
awk '{sum+=$NF}END{printf "Total:\t\t%.6f\n",sum}' $TMP/MNC.txt

echo ""
echo "Pearson of core frequency-vs-biggest: "
wait
pearson -V < $TMP/bigs

