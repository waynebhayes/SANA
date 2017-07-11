#!/bin/sh

USAGE="$0 <multi-align file with K columns> <list of exactly K networks in edgelist format, in the same order the columns of the multi-align file>"

hawk 'ARGIND==1{
	if(ARGIND-2!=net){printf "reading file %s\n",FILENAME; net=ARGIND-2}
	cluster[FNR-1]="\t"$0"\t";
	for(i=1;i<=NF;i++) A[FNR-1][i-1]=$i;
	NC++;
    }
    ARGIND>1{
	if(ARGIND-2!=net){printf "reading file %s\n",FILENAME; net=ARGIND-2;netName[net]=FILENAME}
	node[net][$1]=1;
	node[net][$2]=1;
	edgeList[net][$1][$2]=edgeList[net][$2][$1]=1
	totalEdges++;
    }
    END{
	net++;
	printf "%d total networks with %d total edges\n", net, totalEdges;
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
		    printf "Cluster (%d,%d) = \n\t(%s,\n\t%s)\nE=%d s=%d E*E/s=%g\n", i,j,cluster[i],cluster[j],E,s,E*E/s;
		    cs+=E*E/s
		}
	    };
	printf "totalEdges %d TE %d cs %g cs/TE %g cs/totalEdges %g\n",totalEdges,TE,cs,cs/TE,cs/totalEdges
    }' "$@"
    #/tmp/mp60s/dir-init/AThaliana-CElegans-DMelanogaster-HSapiens-MMusculus-RNorvegicus-SCerevisiae-SPombe.align networks/[A-Z][A-Z][a-z]*/*.el
