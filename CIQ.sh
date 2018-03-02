#!/bin/bash

USAGE="$0 [-V] <multi-align file with K columns> <list of exactly K networks in edgelist format, in the same order the columns of the multi-align file>"

VERBOSE=0

TMPDIR=/tmp/CIQ$$
trap "/bin/rm -rf $TMPDIR; exit" 0 1 2 3 15
mkdir $TMPDIR

case "$1" in
-V) VERBOSE=1; shift ;;
esac

totalNodes=`sed 's/_//g' "$1" | wc -w`
nodesCovered=`sed 's/_//g' "$1" | awk 'NF>1{print}' | wc -w`
NCV=`parse "$nodesCovered/$totalNodes"`
echo "NCV $NCV"

hawk 'ARGIND==1{
	if(ARGIND-2!=net) {
	    if('$VERBOSE')printf "reading file %s\n",FILENAME >"/dev/fd/2";
	    net=ARGIND-2
	}
	cluster[FNR-1]="\t"$0"\t";
	for(i=1;i<=NF;i++) A[FNR-1][i-1]=$i;
	NC++;
    }
    ARGIND>1{
	if(ARGIND-2!=net) {
	    if('$VERBOSE')printf "reading file %s\n",FILENAME >"/dev/fd/2";
	    net=ARGIND-2;netName[net]=FILENAME
	}
	node[net][$1]=1;
	node[net][$2]=1;
	edgeList[net][$1][$2]=edgeList[net][$2][$1]=1
	#printf "%s(%d):%d %s %s\n", FILENAME, net, FNR, $1, $2
	totalEdges++;
    }
    END{
	net++;
	if('$VERBOSE')printf "%d total networks with %d total edges\n", net, totalEdges > "/dev/fd/2";
	for(i=0;i<NC;i++)
	    for(j=i+1;j<NC;j++)
	    {
		#printf "Cluster (%d,%d) = \n\t(%s,\n\t%s)\n", i,j,cluster[i],cluster[j];
		E=s=0;
		for(k=0;k<net;k++)
		    if(A[i][k]!="_"&&A[j][k]!="_")
		    {
			s++;
			if(edgeList[k][A[i][k]][A[j][k]])
			{
			    #printf "edge(%s,%s,%s) exists\n",netName[k],A[i][k],A[j][k]
			    TE++;
			    E++;
			} else {
			    #printf "edge(%s,%s,%s) NOT\n",netName[k],A[i][k],A[j][k]
			}
		    }
		if(E>1)
		{
		    #printf "Cluster (%d,%d) = \n\t(%s,\n\t%s)\nE=%d s=%d E*E/s=%g\n", i,j,cluster[i],cluster[j],E,s,E*E/s;
		    cs+=E*E/s
		}
	    };
	printf "totalEdges %d TE %d cs %g cs/TE %g cs/totalEdges %g\n",totalEdges,TE,cs,cs/TE,cs/totalEdges
	#printf "totalEdges %d CIQ %g\n",totalEdges,cs/TE
    }' "$@" | tee $TMPDIR/CIQ.out
CIQ=`awk '{print $NF}' $TMPDIR/CIQ.out`
echo -n "NCV-CIQ "; echo "sqrt($NCV*$CIQ)"
