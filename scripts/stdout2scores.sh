#!/bin/sh
die() { echo "$@" >&2; exit 1
}

TMP=/tmp/stdout2scores.$$
trap "/bin/rm -f $TMP $TMP.?; exit" 0 1 2 3 15
echo "$@" > $TMP.0

for i
do
    case "$i" in
    *.stdout) ;;
    *) die "expecting a .stdout file!" ;;
    esac

    b=`basename $i .stdout`
    dir=`dirname $i`
    echo -n "$b "
    cp "$i" $TMP
    sed -e '1,/^####/d' -e '/^Executed/,$d' $TMP | tee $TMP.1 | # extract the lines with the core scores
	# The following logic is designed to ensure that each and every pair is output at least once.
	# For every protein p from either network and for every column c, M[p][c]=biggest ever score for p in column c
	# At the end, for each column c, find the *smallest* of all the M[p][c] across all p: this is Smin[c]
	# Finally, output a pair if any of its columns c has score larger than Smin[c]
	    hawk 'NR>1{ # skip the line that has the header
		# For both proteins, update their biggest score ever seen in each column
		for(p=1;p<=2;p++){for(c=3;c<=NF;c++)if($c>M[$p][c])M[$p][c]=$c}
	    }
	    END{ # Now, for each column, find the smallest of the maxima above.
		for(c=3;c<=NF;c++){
		    Smin[c]=1e30
		    for(p in M)if(M[p][c]<Smin[c])Smin[c]=M[p][c]
		    print c,Smin[c]
		}
	    }' > $TMP.2  # this output file contains all the Smin values across columns
    echo -n `awk '{print $2}' $TMP.2` '' # spit Smin values out to the screen
    cat $TMP.2 $TMP.1 | #spit the Smin values, one per line, followed by the core scores that are in $TMP.1
	hawk 'NF==2{Smin[$1]=$2} # record the Smin values
	NF>2&&!/[a-z]/{ #for any line with no letters (eg headers)
	    for(c=3;c<=NF;c++)if($c>=Smin[c]){print;next} #finally, produce the final scores file
	}' | tee $dir/$b.scores | wc -l # The wc -l goes to the screen showing how many core pairs got output
    # Last step: remove the core pairs from the stdout file since they're now in the .scores file.
    sed -n -e '1,/^####/p' -e '/^Executed/,$p' $TMP > $TMP.1; mv $TMP.1 $i
done
