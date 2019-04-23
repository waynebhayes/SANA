#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing SANA use different format as it input.'

TEST_DIR=`pwd`/regression-tests/DifferentFormat
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"

exitCode=0

for network in yeast; do
    file="$TEST_DIR/$network"

    # Run SANA to align the graph to itself
    echo "Aligning network $network in different formats:"
    echo "Aligning EdgeList file with GML file:"
    if ./sana  -fg1 "$file.el" -fg2 "$file.gml" -o "$file" &> "${file}_gml.progress"; then
	:
    else
	echo Edgelist to GML alignment failed
	exitCode=1
    fi

    echo "Aligning EdgeList file with CSV file:"
    if  ./sana  -fg1 "$file.el" -fg2 "$file.csv" -o "$file" &> "${file}_csv.progress"; then
	:
    else
	echo Edgelist to CSV alignment failed
	exitCode=1
    fi

    echo "Aligning EdgeList file with LGF file:"
    if ./sana  -fg1 "$file.el" -fg2 "$file.lgf" -o "$file" &> "${file}_lgf.progress"; then
	:
    else
	echo Edgelist to LGF alignment failed
	exitCode=1
    fi

    echo "Aligning EdgeList file with GraphML file:"
    if ./sana  -fg1 "$file.el" -fg2 "$file.xml" -o "$file" &> "${file}_xml.progress"; then
	:
    else
	echo Edgelist to GraphML alignment failed
	exitCode=1
    fi

done

echo 'Done testing SANA with different formats of input.'
exit $exitCode
