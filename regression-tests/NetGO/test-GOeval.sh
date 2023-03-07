#!/bin/bash
$CI && exit 0 # takes too long for continuous integration
TMPDIR=/tmp/GOeval.$$
 trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
/bin/rm -rf $TMPDIR
mkdir -p $TMPDIR
cp -p go/Entrez/2018/09/*.gz $TMPDIR && gunzip $TMPDIR/*.gz
cp -p regression-tests/NetGO/*.xz $TMPDIR && unxz $TMPDIR/*.xz
rm -f regression-tests/NetGO/MMHS1ec_60.align
gunzip < regression-tests/NetGO/MMHS1ec_60.align.gz > regression-tests/NetGO/MMHS1ec_60.align
EBM=''
$CI || EBM=-ebm # ebm takes too long for continuous integration
if GOeval.sh $EBM -pa MM HS $TMPDIR/MMusculus-3.4.164.el $TMPDIR/HSapiens-3.4.164.el $TMPDIR/go.obo $TMPDIR/gene2go regression-tests/NetGO/MMHS1ec_60.align | fgrep -v '***' | tee $TMPDIR/GOeval.out | sed '/^p-value </s/\.[0-9]*//g' | diff -b <(unxz < regression-tests/NetGO/GOeval.MMHS.correct.xz | sed '/^p-value </s/\.[0-9]*//g') - > $TMPDIR/diff.out; then
    : # do nothing, success
else
    EXIT_CODE="$?"
    echo "diff failed; see $TMPDIR for output; wc: `wc $TMPDIR/diff.out`" >&2
    trap "" 0 1 2 3 15
    exit "$EXIT_CODE"
fi
