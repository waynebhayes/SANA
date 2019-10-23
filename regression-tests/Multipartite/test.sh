#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing measurement CompressedFiles'

TEST_DIR=`pwd`/regression-tests/CompressedFiles
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"

echo "" > $TEST_DIR/sana.align

exitCode=0

echo "Test 1: Testing compressed edgelists AThaliana.el.bz2 and MMusculus.el.gz"
./sana -t 1 -fg1 "$TEST_DIR/multipartite_test.mpel" -fg2 "$TEST_DIR/multipartite_test_2.mpel"  -o "$TEST_DIR/sana.align" -multipartite 3
matches=`awk '{ if ($1 == $2) { print "same"; } }' < sana.align | wc -l`
lines=`wc -l sana.align`
if [ $lines != $matches ]
then
    echo "Test failed to align multipartite graphs"
    exitCode=1
fi


exit $exitCode

