#!/bin/sh

USAGE="$0 [-V] [list of shadow-network alignment files in 2-column format]"

VERBOSE=0

case "$1" in
-V) VERBOSE=1; shift ;;
esac

hawk 'ARGIND>0{
	if(ARGIND!=net){net=ARGIND;netName[net-1]=FILENAME;if('$VERBOSE')printf "reading file %s\n",FILENAME >"/dev/fd/2"}
	if(index($1,"shadow")>0)
	    {shadowNode=$1;mappedNode=$2}
	else #if(index($2,"shadow")>0)
	    {shadowNode=$2;mappedNode=$1}
	#else
	#    Fatal(sprintf("%s: no shadow node listed on line %d (%s)\n",FILENAME,FNR,$0))
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
