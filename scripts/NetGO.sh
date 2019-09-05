#!/bin/sh
USAGE="Expected input: a file of clusters, 1 per line, gene2go file LAST. Output: the same line, preceded by K(C) (not necessarily in the same order), and then a final line with the total knowledge, and then the K(A) of the 'alignment'."

die() { echo "$@" >&2; exit 1
}

minW=0
minK=0
minA=0

while :; do
    case "$1" in
    -min[wW]*) minW=$2; shift 2;;
    -min[kK]*) minK=$2; shift 2;;
    -min[aA]*) minA=$2; shift 2;;
    -*) die "unknown option '$1'" ;;
    *) break ;;
    esac
done

hawk '
    !/GO:/&&!/tax_id.*PubMed/{ # assume a cluster line
	for(i=1;i<=NF;i++) {
	    P[$i]=1; # mark this protein as one whose GO terms we need.
	    C[NR][$i]=1; # define membership in the cluster on this line
	}
	next; # skip the possibility of this being a gene2go file
    }
    FNR>1&& 	# ignore top (header) line of gene2go file
	$2 in P{# and only consider GO terms that annotate proteins we have seen
	++GOfreq[$3] # increase the frequency of this GO term by 1
	pGO[$2][$3]=1 # keep track of the GO terms that annotate this protein
    }
    END{
	for(u in P)
	    if(u in pGO) for(g in pGO[u]) K[u]+=1/GOfreq[g]
	    else{
		K[u]=0;
		pGO[u][1]=0; # dummy assignment so that...
		delete pGO[u][1] # we can make pGO[u] an empty array
	    }
	totalK = 0;
	for(c in C){
	    delete numP # this will count the number of proteins annotated by each GO term that appears in this cluster
	    for(u in C[c]) if(K[u]>='$minK') for(g in pGO[u]) ++numP[g]; #count frequency of each GO term in this cluster
	    totalGOs=length(numP) # total number of GO terms that appear in this cluster
	    # At this point, if we are as Draconian as possible, the only GO terms that are in the intersection across
	    # all proteins are those for which numP[g] = length(c), ie, this GO term appeared in every protein.
	    # We allow less Draconian interpretations by allowing minK>0.
	    K_C = 0;
	    for(g in numP) if(numP[g] >= length(C[c]) - '$minA') K_C += 1/GOfreq[g];
	    totalK += K_C
	    printf "%g", K_C
	    for(u in C[c]) printf "\t%g %s", K[u], u
	    print ""
	}
	printf "%g %d %d %g (total common knowledge,numGO,numClust,similarity K/numGO)\n",
	    totalK, length(GOfreq), length(C), totalK/length(GOfreq)
    }' "$@"
