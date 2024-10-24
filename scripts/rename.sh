#!/bin/bash
################## SKELETON: DO NOT TOUCH THESE 2 LINES
EXEDIR=`dirname "$0"`; BASENAME=`basename "$0" .sh`; TAB='	'; NUL=" "; NL='
'
#################### ADD YOUR USAGE MESSAGE HERE, and the rest of your code after END OF SKELETON ##################
USAGE="USAGE: $BASENAME [-n] [-i] [-f] ';from;to;' FILENAME(s)
PURPOSE: algorithmically rename a list of files using the sed regexp ';from;to;'
OPTIONS:
    -n: only print what WOULD happen, don't actually rename anything
    -f: force (don't ask about overwriting any destination files that already exists)
    -i: opposite of '-f': ask for every overwrite."

################## SKELETON: DO NOT TOUCH CODE HERE
# check that you really did add a usage message above
USAGE=${USAGE:?"$0 should have a USAGE message before sourcing skel.sh"}
die(){ echo "$USAGE${NL}FATAL ERROR in $BASENAME:" "$@" >&2; exit 1; }
[ "$BASENAME" == skel ] && die "$0 is a skeleton Bourne Shell script; your scripts should source it, not run it"
echo "$BASENAME" | grep "[ $TAB]" && die "Shell script names really REALLY shouldn't contain spaces or tabs"
[ $BASENAME == "$BASENAME" ] || die "something weird with filename in '$BASENAME'"
warn(){ (echo "WARNING: $@")>&2; }
not(){ if eval "$@"; then return 1; else return 0; fi; }
newlines(){ awk '{for(i=1; i<=NF;i++)print $i}' "$@"; }
parse(){ awk "BEGIN{print $*}" </dev/null; }

# Temporary Filename + Directory (both, you can use either, note they'll have different random stuff in the XXXXXX part)
[ "$MYTMP" ] || MYTMP=`for i in /scratch/preserve/$USER /var/tmp/$USER /tmp/$USER; do mkdir -p $i 2>/dev/null && break; done; echo $i`
TMPDIR=`mktemp -d $MYTMP/$BASENAME.XXXXXX`
 trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15 # call trap "" N to remove the trap for signal N

#################### END OF SKELETON, ADD YOUR CODE BELOW THIS LINE

[ $# -ge 1 ] || die "expecting at least 1 argument (filename list is allowed to be empty)"

# check for options
ASK='-n' # default to not clobber
ECHO_ONLY=false
case "$1" in
-i) ASK='-i'; shift;; # interactive
-f) ASK=''; shift;;
-n) ECHO_ONLY=true; shift;;
esac

# check for erroneous use of / in sed separator.
case "$1" in
*/*) die "Use ; instead of / as separator";;
\;*\;*\; ) ;; # this is what we want
\;*\;*\;g) ;; # this is what we want
[^';']*) die "substitution must have 3 semicolons now, not just 1" ;;
*) die "unknown replacement command" ;;
esac

pattern="$1"
shift
for i
do
    case "$pattern" in
	# second case handles a "once per file" substitution; you don't
	# supply the outside slashes, this pattern does it for you.
	*) newname=`echo "$i" | sed "s$pattern"`;;
    esac
    case "$newname" in
	"$i") ;;    # same name, don't do anything, else mv(1) will complain.
	*) if "$ECHO_ONLY"; then
	    echo mv $ASK "$i" "$newname"
	   else
	    mv $ASK "$i" "$newname"
	   fi
	;;
    esac
done
