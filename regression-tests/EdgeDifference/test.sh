#!/bin/bash

echo 'Testing measurement Edge Difference'

CURRENT_DIR=`pwd`/regression-tests/EdgeDifference

for network in 150 WMean_con WMean_ocd; do
    file="$CURRENT_DIR/$network"

    # Run SANA to align the graph to itself
    echo "Aligning network $network"
    ./sana -t 1 -fg1 "$file.elw" -fg2 "$file.elw" -ed 1 -s3 0 -o "$file" &> "$file.progress"

    # Test if SANA has aligned the graph to itself
    match_nodes=`awk '$1==$2' $file.align | wc -l`
    nodes_count=`awk '{ print $1"\n"$2 }' $file.elw | sort | uniq | wc -l`
    if (( $match_nodes != $nodes_count )); then
        echo "Failed to align $network to itself" 
        echo "  EXPECTED matching nodes: $nodes_count GOT: $match_nodes"
        exit 1
    fi
    
    # Remove files if test is passed for this network.
    for file in $file.align $file.out $file.progress; do
      rm $file
    done 
done

echo 'Testing measurement Edge Difference passed.'
