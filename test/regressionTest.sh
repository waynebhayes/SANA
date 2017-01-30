#!/bin/bash

olddata="regression_test_baseline.txt"

args="
  -seed 10000
  -g1 yeast
  -g2 human
  -nodecweights 4 0.1 0.25 0.5 0.15
  -edgecweights 4 0.1 0.25 0.5 0.15
  -goweights 1 1
  -wecnodesim graphletlgraal
  -tinitial 1
  -tdecay 1
  -t 1
  -method sana
  -ec 1
  -lgraaliter 1000
  -alpha 0
  -tnew 3
  -iterperstep 10000000
  -numcand 3
  -tcand 1
  -tfin 3
  -qcount 1
  -o regression
  -rewire 0
"

echo "Running SANA regression test"

make #> /dev/null
./sana $args 2>&1 | tee run_output.txt
#rm x

# Only look at the first 7 iterations because not all computers will get through the same number
# of iterations
scores=(`grep -aA7 "s): " run_output.txt  | sed -e 's/.*score = \(.*\) P(.*/\1/'`)

if [[ ! -e $olddata ]]
then
	echo "Run saved as baseline."
	echo ${scores[*]} | tr " " "\n" > $olddata
	echo $(date) >> $olddata
else
	oldrun=(`cat $olddata`)
	oldScores=(${oldrun[@]:0:6})
	olddate=${oldrun[7]}

	isAtLeastOneLessThanTolerance=false

	i=0
	while [ $i -lt 6 ]; do
		if [ $(echo "${scores[$i]} < ${oldScores[$i]}" | bc) == "1" ]; then
			echo "Less than oldscore on the "$i"th iteration."
			echo "${scores[$i]} < ${oldScores[$i]}"
			isAtLeastOneLessThanTolerance=true
		fi
		let i=i+1
	done

	if [ isAtLeastOneLessThanTolerance == true ]; then
		echo "Preformed better on the "$olddata
	else
		echo "All good!"
		echo ${scores[*]} | tr " " "\n" > $olddata
		echo $(date) >> $olddata
	fi
fi

#rm run_output.txt

