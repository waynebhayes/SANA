#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing Multipartite mode'

TEST_DIR=`pwd`/regression-tests/Multipartite
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"


rm -f $TEST_DIR/sana.align

exitCode=0

./sana -s3 1 -t 1 -fg1 "$TEST_DIR/multipartite_test.mpel" -fg2 "$TEST_DIR/multipartite_test_2.mpel" -o "$TEST_DIR/sana" -multipartite 3 || exit

cd $TEST_DIR
matches=`awk '{ if ($1 == $2) { print "same"; } }' < sana.align | wc -l`
lines=`wc -l < sana.align`

if [ $lines != $matches ]
then
    echo "Test failed to align multipartite graphs"
    exitCode=1
fi

exit $exitCode
