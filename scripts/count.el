#!/bin/sh
TYPES=false
case $1 in
-nodes-have-types) TYPES=true; shift ;;
*) ;;
esac

case $# in
0) set - -;; # read the standard input if there's no arguments
esac

DOS2UNIX='BEGIN{CR=sprintf("%c",13)}{sub(CR,"")}' # 0xD is ASCII 13, carriage-return
for i
do
    case "$i" in
    *.gw) awk "$DOS2UNIX"'oneField==1&&number>0&&/{.*}/{printf "%d\t",number}{oneField=0}NF==1{oneField=1;number=1*$1}END{print FILENAME}' "$i" ;;
    *)
	if $TYPES; then
	    awk "$DOS2UNIX"'{node1[$1]=node2[$2]=1;numEdges++}END{for(i in node1)numNodes1++; for(i in node2)numNodes2++; print numNodes1, numNodes2, numEdges, FILENAME}' "$i"
	else
	    awk "$DOS2UNIX"'
	    NF==1{
		if(FNR==1)n=1*$1; else if(FNR==2)m=1*$1;else {
		    printf "can only have 1 column in first 2 lines, not line %d\n",FNR >"/dev/stderr";
		    exit(1)
		}
		next;
	    }
	    NF==2{node[$1]=node[$2]=1;next}
	    {printf "wrong number of fields %d on line %d, line is \"%s\"\n",NF,FNR,$0 >"/dev/stderr"}
	    END{
		if((n||m) && length(node)!=FNR-!!n-!!m)
		    printf "mismatch between number of nodes: first line says %d, but length(node) is %d\n",
			n, length(node) > "/dev/stderr";
		else printf "%d\t%d\t%s\n", length(node), NR-!!n-!!m, FILENAME
	    }' "$i"
	fi
	;;
    esac
done
