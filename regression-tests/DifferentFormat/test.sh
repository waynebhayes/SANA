#!/bin/bash
die() { echo "$@" >&2; exit 1
}
CORES=$((${CORES:=`./scripts/cpus 2>/dev/null || echo 4`}-1))

echo 'Testing SANA use different format as it input.'

REG_DIR=`pwd`/regression-tests/DifferentFormat
[ -d "$REG_DIR" ] || die "should be run from top-level directory of the SANA repo"
[ -x "$SANA_EXE" ] || die "can't find executable '$SANA_EXE'"

NETS=yeast
#TYPES='gml csv lgf xml'
TYPES='csv lgf xml'
echo "****************************** WARNING: IGNORING GML TEST FOR NOW *************************"
echo "****************************** WARNING: IGNORING GML TEST FOR NOW *************************"
echo "****************************** WARNING: IGNORING GML TEST FOR NOW *************************"
echo "****************************** WARNING: IGNORING GML TEST FOR NOW *************************"
echo "****************************** WARNING: IGNORING GML TEST FOR NOW *************************"

for network in $NETS; do
    file="$REG_DIR/$network"
    for type in $TYPES; do
	echo "'$SANA_EXE' -s3 1 -t 1 -fg1 '$file.el' -fg2 '$file.$type' -o '$file' &> '${file}_$type.progress'"
    done
done | ./parallel -s /bin/bash $CORES
status=$?
echo -n "Out of `ls $REG_DIR/*.progress|wc -l` tries, numSuccesses was "
cat $REG_DIR/*.progress | fgrep -c 'Saving report as '
exit $status
