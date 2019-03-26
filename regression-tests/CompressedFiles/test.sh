#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing measurement CompressedFiles'

TEST_DIR=`pwd`/regression-tests/CompressedFiles
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"

echo "" > test1.align
echo "" > test2.align

exitCode=0

echo "Test 1: Testing compressed edgelists AThaliana.el.bz2 and MMusculus.el.gz"
./sana -t 1 -fg1 "$TEST_DIR/AThaliana.el.bz2" -fg2 "$TEST_DIR/MMusculus.el.gz"  -o "test1" &> "test1.progress"
lines=`wc -l test1.align | awk '{print $1}'`;
if [ $lines -lt 100 ]
then
    echo "Test 1: Failed to load networks AThaliana.el.bz2 and MMusculus.el.gz"
    exitCode=1
fi


echo "Test 2: Testing compressed edgelists AThaliana.el.gz and yeast-reduced-wayne.gw.gz"
./sana -t 1 -fg1 "$TEST_DIR/AThaliana.el.gz" -fg2 "$TEST_DIR/yeast-reduced-wayne.gw.gz"  -o "test2" &> "test2.progress"
lines=`wc -l test1.align | awk '{print $1}'`;
if [ $lines -lt 100 ]
then
    echo "Test 2: Failed to load networks AThaliana.el.gz and yeast-reduced-wayne.gw.gz"
    exitCode=1
fi

echo 'Done testing compression'
exit $exitCode

