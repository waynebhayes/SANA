#!/bin/bash
die() { echo "$@" >&2; exit 1
}

echo 'Testing measurement CompressedFiles'

[ -d "$REG_DIR" ] || die "should be run from top-level directory of the SANA repo"
[ -x "$EXE" ] || die "can't find executable '$EXE'"

echo "" > $REG_DIR/test1.align
echo "" > $REG_DIR/test2.align

NUM_FAILS=0

echo "Test 1: Testing compressed edgelists AThaliana.el.bz2 and MMusculus.el.gz"
"$EXE" -s3 1 -tolerance 0 -t 1 -fg1 "$REG_DIR/MMusculus.el.gz" -fg2 "$REG_DIR/AThaliana.el.bz2" -o "$REG_DIR/test1" &> "$REG_DIR/test1.progress" &
echo "Test 2: Testing compressed edgelists AThaliana.el.gz and yeast-reduced-wayne.gw.gz"
"$EXE" -s3 1 -tolerance 0 -t 1 -fg1 "$REG_DIR/yeast-reduced-wayne.gw.gz" -fg2 "$REG_DIR/AThaliana.el.gz" -o "$REG_DIR/test2" &> "$REG_DIR/test2.progress"

wait

lines=`wc -l $REG_DIR/test1.align | awk '{print 100-$1}'`;
if [ $lines -gt 0 ]
then
    echo "Test 1: Failed to load networks AThaliana.el.bz2 and MMusculus.el.gz, missed lines=$lines"
    (( NUM_FAILS+=$lines ))
fi

lines=`wc -l $REG_DIR/test1.align | awk '{print 100-$1}'`;
if [ $lines -gt 0 ]
then
    echo "Test 2: Failed to load networks AThaliana.el.gz and yeast-reduced-wayne.gw.gz, missed lines=$lines"
    (( NUM_FAILS+=$lines ))
fi

echo 'Done testing compression'
exit $NUM_FAILS

