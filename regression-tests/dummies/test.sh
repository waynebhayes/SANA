#!/bin/sh
die() { echo "FATAL ERROR: $@" >&2; exit 1
}
TMPDIR=/tmp/regression-dummies.$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir $TMPDIR

[ -x "$EXE" ] || die "can't find executable '$EXE'"
echo "The following SHOULD abort, not to worry:" >&2
if ./$EXE -fg1 $REG_DIR/testG1.el -fg2 $REG_DIR/testG2.el -fcolor1 $REG_DIR/testG1.col -fcolor2 $REG_DIR/testG2.col -ec 1 -t 0.1 -tinitial 1 -tdecay 1 > $TMPDIR/sana.stdout 2>&1; then
    cat $TMPDIR/sana.stdout >&2
    echo 'Hmmm... this one was supposed to fail' >&2
    exit 1
else
    exit 0
fi
# adding 2 dummies colored red to G2
# adding 1 dummies colored blue to G2
# adding 1 dummies colored __default to G2
# Warning: some G2 nodes have a color non-existent in G1, so some G2 nodes won't be part of any valid alignment

#sana.align:
# 0       dummy_0_red
# 1       15
# 2       14
# 3       dummy_1_red
# 4       dummy_0_blue    <-- only choice for 4 
# 5       dummy_0         <-- only choice for 5
