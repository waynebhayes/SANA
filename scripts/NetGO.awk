#!/bin/sh
USAGE="$0 [-D] alignFile gene2goFile
    -D: 'Draconian', which insists a GO term annotating every protein in a cluster for it to count.
	By default, a GO term's weight per-cluster is scaled by the number of proteins (>1 at least) it annotates.
    alignFile: each line consists of a cluster of any number of proteins; proteins can appear in more than one cluster.
    gene2goFile: a standard-format gene2go file downloaded from the GO consortium's website.
"
die() { echo "$USAGE" >&2; echo "$@" >&2; exit 1
}
# Note awk cannot return arrays, so anything expecting an array as output must give that array as the first argument,
# which is passed by reference, and which will contain the result.
# awk global variables:
# pGO: an array pGO[proteinName string][GOname string]=1 if protein p (string) is annotated by GO term g (string).

# Coding convention: awk allows no local variables other than parameters, but it also allows you to declare more
# parameters than are passed. Thus the convention is to declare local variables as extra parameters, as the second
# line of the parameter declarations.

DRACONIAN=0
case "$1" in
-D*) DRACONIAN=1;shift;;
-*) die "unknown option '$1'";;
esac
[ $# -eq 2 ] || die "expecting 2 arguments, alignFile and gene2goFile"

hawk '
function SetIntersect(res1,T1,T2,
    g){delete res1;if(length(T1)<length(T2)){for(g in T1)if(g in T2)res1[g]=1}
                                                       else{for(g in T2)if(g in T1)res1[g]=1}}
function SetUnion(res2,T1,T2,
    g){delete res2;for(g in T1)res2[g]=1;for(g in T2)res2[g]=1}
function SetCumulativeUnion(res2,T1, g){for(g in T1)res2[g]=1}

function K_g(g){if(g in GOp) return 1/length(GOp[g]); else return 0}

function K_gset(T,
    g,sum){sum=0;for(g in T)sum+=K_g(g); return sum}

function K_p(p,
    sum){sum=0;if(p in pGO)return K_gset(pGO[p]); else return 0}

# Knowledge in a simple pairwise alignment in which A[u]=v
function K_A2(A,
    sum,u,v,Tuv){sum=0;for(u in A)if(u in pGO){v=A[u];if(v in pGO){SetIntersect(Tuv,pGO[u],pGO[v]); sum+=K_gset(Tuv)}}; return sum}

# Knowledge in a general alignment of protein clusters
function K_AC(C,
    sum,numProteins,numA,cl,i,u,g,T,M,summand){
    sum=0;
    for(cl in C){
	summand=0;
	#printf "Cluster %d, length %d;",cl,length(C[cl])
	numProteins=length(C[cl]);
	delete M; M[0]=1;delete M[0]; # initialize M to empty list; M=set of protein members;
	delete T; T[0]=1;delete T[0]; # initialize T to empty list; T=set of GO terms across the cluster;
	delete numA; numA[0]=1; delete numA[0]; # array of how many proteins each GO term annotates.
	if(numProteins>1){ # have to match more than one protein to be interesting.
	    # initialize T to the GO terms of first protein in the cluster:
	    u=C[cl][1]; if(u!="_"&&u!="NA"){++M[u];for(g in pGO[u]){T[g]=1;++numA[g]}}
	    for(i=2;i<=numProteins;i++){u=C[cl][i];if(u=="_"||u=="NA")continue;
		++M[u];
		if(DRACONIAN) {
		    for(g in T)if(!(g in pGO[u]))delete T[g]
		} else {
		    for(g in T){T[g]=1;++numA[g]}
		}
	    }
	    #printf " col %d=%s, |T|=%d", i, u,length(T)
	}
	if(length(T)>0 && # if this cluster has any annotations...
	    (length(M)>1) || # and it has more than one protein...
		(length(M)==1 && M[u]>1)) # or the same protein multiple times...
	{
	    if(DRACONIAN) summand+=K_gset(T)
	    else {
		for(i in M)numProteins+=(M[i]-1)
		for(g in T)if(numA[g]>1)summand+=numA[g]*K_g(g)/numProteins
	    }
	}
	#printf " K=%g\n",summand
	sum+=summand
    }
    return sum
}
function K_C(C,
    sum,T){
    sum=0
    delete T; T[0]=1;delete T[0]; #make T the empty set
}

# Knowledge in a alignment of a bunch of clusters
function sim_A2(A){return K_A2(A)/length(GOp)}

#Old only-2-column alignment files
#ARGIND==1{C[$1]=$2; C_[$2]=$1}
#ARGIND==2&&($2 in C || $2 in C_){pGO[$2][$3]=1;++GOp[$3][$2]}

BEGIN{DRACONIAN='$DRACONIAN'}
#Clusters version
ARGIND==1{for(i=1;i<=NF;i++){CA[FNR][i]=$i;++pC[$i][FNR]}} # CA[][]=cluster alignment; pC[p] = clusters this protein is in.
ARGIND==2&&($2 in pC){pGO[$2][$3]=1;++GOp[$3][$2]}
END{for(p in pC)if(!(p in pGO)){pGO[p][0]=1;delete pGO[p][0]} #proteins with no GO terms need pGO[p] explicit empty list.
    know=K_AC(CA); printf "%d %d %d %g %g\n", length(CA), length(pGO), length(GOp), know, know/length(GOp)
}' "$@"
