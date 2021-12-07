#!/bin/sh
DIR=/tmp/NetGO.$$
/bin/rm -rf $DIR
mkdir -p $DIR
trap "/bin/rm -rf $DIR; exit" 0 1 2 3 15
cp -p go/Entrez/2018/09/*.gz $DIR
gunzip $DIR/*.gz
echo Testing NetGO on regression-tests/multi-pairwise/BioGRID-100x1-dir300-multiAlign.tsv.xz
unxz < regression-tests/multi-pairwise/BioGRID-100x1-dir300-multiAlign.tsv.xz > $DIR/BG.multi.tsv
./NetGO/NetGO.awk $DIR/go.obo $DIR/gene2go $DIR/BG.multi.tsv > $DIR/NetGO.out
STATUS=$?
echo -n "md5sum of output: "; md5sum $DIR/NetGO.out
tail -1 $DIR/NetGO.out
