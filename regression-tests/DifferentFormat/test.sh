#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing SANA use different format as it input.'

TEST_DIR=`pwd`/regression-tests/DifferentFormat
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"
TYPES='gml csv lgf xml'
NETS=yeast
for network in $NETS; do
    file="$TEST_DIR/$network"
    for type in $TYPES; do
	echo "./sana -s3 1 -t 1 -fg1 '$file.el' -fg2 '$file.$type' -o '$file' &> '${file}_$type.progress'"
    done
done | ./parallel -s /bin/bash 4
status=$?
echo -n "The number of successful completions was ";
cat $TEST_DIR/*.progress | fgrep -c 'Saving report as'
exit $status
