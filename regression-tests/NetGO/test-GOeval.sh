#!/bin/sh
DIR=/tmp/GOeval.$$
/bin/rm -rf $DIR
mkdir -p $DIR
trap "/bin/rm -rf $DIR; exit" 0 1 2 3 15
cp -p go/Entrez/2018/09/*.gz $DIR && gunzip $DIR/*.gz
cp -p regression-tests/NetGO/*.xz $DIR && unxz $DIR/*.xz
GOeval.sh MM HS $DIR/MMusculus-3.4.164.el $DIR/HSapiens-3.4.164.el $DIR/go.obo $DIR/gene2go regression-tests/NetGO/MMHS1ec_60.align | fgrep -v '***' | diff -b - regression-tests/NetGO/GOeval.MMHS.correct
