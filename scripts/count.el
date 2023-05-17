#!/bin/sh
TYPES=false
case $1 in
-nodes-have-types) TYPES=true; shift ;;
*) ;;
esac

case $# in
0) set - -;; # read the standard input if there's no arguments
esac

DOS2UNIX='{sub(0xD,"")}' # 0xD is ASCII 13, carriage-return
for i
do
    case "$i" in
    *.gw) awk "$DOS2UNIX"'oneField==1&&number>0&&/{.*}/{printf "%d\t",number}{oneField=0}NF==1{oneField=1;number=1*$1}END{print FILENAME}' "$i" ;;
    *)
	if $TYPES; then
	    awk "$DOS2UNIX"'{node1[$1]=node2[$2]=1;numEdges++}END{for(i in node1)numNodes1++; for(i in node2)numNodes2++; print numNodes1, numNodes2, numEdges, FILENAME}' "$i"
	else
	    awk "$DOS2UNIX"'{node[$1]=node[$2]=1}END{printf "%d\t%d\t%s\n", length(node), NR, FILENAME}' "$i"
	fi
	;;
    esac
done
