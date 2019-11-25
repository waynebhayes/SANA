#!/bin/sh
# Actually, I lied.  The solution to the exam question is in "rename.209".
# This one is more complicated than what was necessary for the exam.
die() { echo "$@" >&2; exit 1
}

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
