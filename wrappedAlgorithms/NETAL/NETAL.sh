#!/bin/sh
die(){ echo "$@" >&2; exit 1
}
G1="$1"
G2="$2"
shift 2

S1=`echo $G1 | sed 's/.*_//'`
S2=`echo $G2 | sed 's/.*_//'`
[ -d ../../networks/$S1 ] || die "network $S1 does not exist"
[ -d ../../networks/$S2 ] || die "network $S2 does not exist"

# Make the required sequence symbolic links (even though this run may not use them)
(
    ln -s sequence/$S1-$S1.val $G1-$G1.val
    ln -s sequence/$S2-$S2.val $G2-$G2.val
    ln -s sequence/$S1-$S2.val $G1-$G2.val
) 2>/dev/null
exec ./NETAL "$G1" "$G2" "$@"
