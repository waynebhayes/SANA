#!/bin/sh

USAGE="$0 [list of shadow-network alignment files in 2-column format]"

hawk 'function Fatal(s){printf "FATAL ERROR: %s\n",s>"/dev/fd/2"; exit(1);}
    ARGIND>0{
	if(ARGIND!=net){printf "reading file %s\n",FILENAME >"/dev/fd/2"; net=ARGIND;netName[net-1]=FILENAME}
	if(index($1,"shadow")>0)
	    {shadowNode=$1;mappedNode=$2}
	else if(index($2,"shadow")>0)
	    {shadowNode=$2;mappedNode=$1}
	else
	    Fatal(sprintf("%s: no shadow node listed on line %d (%s)\n",FILENAME,FNR,$0))
	if(shadowMap[shadowNode][net-1])
	    Fatal(sprintf("%s: shadow node %s already has value %s on on line %d (%s)\n",FILENAME,shadowNode,shadowMap[shadowNode][net-1],FNR,$0))
	shadowMap[shadowNode][net-1]=mappedNode
	if(shadowNodes[shadowNode]){}
	else {nShadowNodes++; shadowNodes[shadowNode]=1}
    }
    END{
	for(s in shadowNodes){
	    lineOut=""
	    for(i=0;i<net;i++)
		lineOut=lineOut sprintf("%s\t", shadowMap[s][i]?shadowMap[s][i]:"_")
	    gsub("\t$","",lineOut)
	    print lineOut
	}
    }' "$@"

exit


	net++;
	printf "%d total alignment files %d\n", net;
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
