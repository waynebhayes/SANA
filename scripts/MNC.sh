#!/bin/sh
USAGE="MNC=Multi-Node-Correctness"
TMP=/tmp/MNC.$$
trap "/bin/rm -f $TMP; exit" 0 1 2 3 15

cat "$@" | sort -S1G | uniq -c | awk ' { delete K; for(i=2;i<=NF;i++)++K[$i];delete K["_"];biggest=0;for(i in K)if(K[i]>biggest)biggest=K[i];print $1,biggest}' > $TMP & # get this running so pearson will be fast later

for i
do
    echo -n "$i	"
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
    END{printf "%d\t%.3f\t%.3f\n",NR,wSumNC/weight,correct/possible}' "$i"
done
echo "Pearson of core frequency-vs-biggest: "
wait
pearson < $TMP

