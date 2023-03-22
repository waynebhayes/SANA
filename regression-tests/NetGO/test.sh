#!/bin/sh
DIR=/tmp/NetGO.$$
/bin/rm -rf $DIR
mkdir -p $DIR
trap "/bin/rm -rf $DIR" 0 1 2 3 15
cp -p resnik/2018/*.gz $DIR
gunzip $DIR/*.gz
unxz < regression-tests/multi-pairwise/BioGRID-100x1-dir300-multiAlign.tsv.xz > $DIR/BG.multi.tsv
./NetGO/NetGO.awk $DIR/go.obo $DIR/gene2go $DIR/BG.multi.tsv | egrep -v '^---' | sed 's,^/tmp/.*: ,,' > $DIR/test.out
if unxz < regression-tests/NetGO/MMHS1ec_60-correct.out.xz | cmp - $DIR/test.out; then
    echo passed
else
    echo "failed; see output in $DIR"
    unxz < regression-tests/NetGO/MMHS1ec_60-correct.out.xz > $DIR/correct.out
    trap "" 0 1 2 3 15
    exit 1
fi
