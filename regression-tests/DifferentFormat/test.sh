#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing measurement Edge Difference'

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
    ./sana  -fg1 "$file.el" -fg2 "$file.csv" -o "$file" &> "${file}_csv.progress"
    echo "Aligning success."
    echo "Aligning EdgeList file with LGF file:"
    ./sana  -fg1 "$file.el" -fg2 "$file.lgf" -o "$file" &> "${file}_lgf.progress"
    echo "Aligning success."
    echo "Aligning EdgeList file with GraphML file:"
    ./sana  -fg1 "$file.el" -fg2 "$file.xml" -o "$file" &> "${file}_xml.progress"
    echo "Aligning success."

done

echo 'Done testing SANA with different formats of input.'
exit $exitCode
