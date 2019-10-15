#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing SANA use different format as it input.'

TEST_DIR=`pwd`/regression-tests/DifferentFormat
[ -d "$TEST_DIR" ] || die "should be run from top-level directory of the SANA repo"

for network in yeast; do
    file="$TEST_DIR/$network"
    echo "./sana -s3 1 -t 1 -fg1 '$file.el' -fg2 '$file.gml' -o '$file' &> '${file}_gml.progress'"
    echo "./sana -s3 1 -t 1 -fg1 '$file.el' -fg2 '$file.csv' -o '$file' &> '${file}_csv.progress'"
    echo "./sana -s3 1 -t 1 -fg1 '$file.el' -fg2 '$file.lgf' -o '$file' &> '${file}_lgf.progress'"
    echo "./sana -s3 1 -t 1 -fg1 '$file.el' -fg2 '$file.xml' -o '$file' &> '${file}_xml.progress'"
done | parallel -s /bin/bash 4
exit $?
