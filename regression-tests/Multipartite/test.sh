#!/bin/bash
die() { /bin/echo "$@" >&2; exit 1
}

/bin/echo -n 'Testing Multipartite mode: '
CORES=`cpus 2>/dev/null || echo 4`

TEST_DIR=`pwd`/regression-tests/Multipartite
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"

rm -f $TEST_DIR/mpel.align $TEST_DIR/mpel.*out

./sana -s3 1 -t 1 -fg1 "$TEST_DIR/multipartite_test.mpel" -fg2 "$TEST_DIR/multipartite_test_2.mpel" -o "$TEST_DIR/mpel" -multipartite 3 >$TEST_DIR/mpel.stdout 2>&1 || die "sana failed to run"

cd $TEST_DIR
matches=`awk '{ if ($1 == $2) { print "same"; } }' < mpel.align | wc -l`
lines=`wc -l < mpel.align`

if [ $lines -eq $matches ]; then
    echo "multi-partite test passed"
else
    die "Test failed to align multipartite graphs"
fi
