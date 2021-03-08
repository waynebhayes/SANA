#!/bin/sh
USAGE="`basename $0` numRuns 'objective' baseOutDir [networks*.el]
PURPOSE: Generate numRuns multi-pairwise.sh command lines applied to the same set of networks, optimizing objective, with all going to baseOutDir/subdir"

# generally useful Variables
NL='
'
TAB='	'

# Functions
die(){ (echo "USAGE: $USAGE"; echo "${NL}FATAL ERROR in `basename $0`: $@")>&2; exit 1; }
warn(){ (echo "WARNING: $@")>&2; }
not(){ if eval "$@"; then return 1; else return 0; fi; }
newlines(){ awk '{for(i=1; i<=NF;i++)print $i}' "$@"; }
parse(){ awk "BEGIN{print $@}" </dev/null; }

BASENAME=`basename "$0" .sh`
[ $BASENAME == "$BASENAME" ] || die "something weird with filename in '$BASENAME'"

# Temporary Filename + Directory (both, you can use either, note they'll have different random stuff in the XXXXXX part)
TMPDIR=`mktemp -d /tmp/$BASENAME.XXXXXX`
trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15 # call trap "" N to remove the trap for signal N

N=$1; [ "$N" -ge 1 ] || die "expecting N>=1"
obj="$2"
baseDir="$3"
shift 3

yes "./multi-pairwise.sh -frugal -7za ./sana.multi '$obj' 1024 1 0 '$baseDir/%02d'" "$@" | head -$N | awk '{fmt=$0"; /bin/rm core.*\n"; printf fmt, NR-1}'
