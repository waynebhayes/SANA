#!/bin/sh
USAGE="$0 [-Drac] alignFile gene2go"
# Note awk cannot return arrays, so anything expecting an array as output must give that array as the first argument,
# which is passed by reference, and which will contain the result.
# awk global variables:
# pGO: an array pGO[proteinName string][GOname string]=1 if protein p (string) is annotated by GO term g (string).
# GOfreq[GOterm g]: the (global) frequency of GO term g.

# Coding convention: awk allows no local variables other than parameters, but it also allows you to declare more
# parameters than are passed. Thus the convention is to declare local variables as extra parameters, as the second
# line of the parameter declarations.

DRACONIAN=0
case "$1" in
-D*) DRACONIAN=1;shift;;
esac
hawk '
function SetIntersect(res1,T1,T2,
    g){delete res1;if(length(T1)<length(T2)){for(g in T1)if(g in T2)res1[g]=1}
                                                       else{for(g in T2)if(g in T1)res1[g]=1}}
function SetUnion(res2,T1,T2,
    g){delete res2;for(g in T1)res2[g]=1;for(g in T2)res2[g]=1}
function SetCumulativeUnion(res2,T1, g){for(g in T1)res2[g]=1}

function K_g(g){if(g in GOfreq) return 1/GOfreq[g]; else return 0}

function K_gset(T,
    g,sum){sum=0;for(g in T)sum+=K_g(g); return sum}

function K_p(p,
    sum){sum=0;if(p in pGO)return K_gset(pGO[p]); else return 0}

# Knowledge in a simple pairwise alignment in which A[u]=v
function K_A2(A,
    sum,u,v,Tuv){sum=0;for(u in A)if(u in pGO){v=A[u];if(v in pGO){SetIntersect(Tuv,pGO[u],pGO[v]); sum+=K_gset(Tuv)}}; return sum}

# Knowledge in a general alignment of protein clusters
function K_AC(C,
    sum,numProteins,numA,cl,i,u,g,T,M){
    sum=0;
    for(cl in C){
	#printf "Cluster %d, length %d;",cl,length(C[cl])
	numProteins=length(C[cl]);
	delete M; M[0]=1;delete M[0]; # initialize M to empty list; M=set of protein members;
	delete T; T[0]=1;delete T[0]; # initialize T to empty list; T=set of GO terms across the cluster;
	delete numA; numA[0]=1; delete numA[0]; # array of how many proteins each GO term annotates.
	if(numProteins>1){ # have to match more than one protein to be interesting.
	    # initialize T to the GO terms of first protein in the cluster:
	    u=C[cl][1]; if(u!="_"&&u!="NA"){M[u]=1;for(g in pGO[u]){T[g]=1;++numA[g]}}
	    for(i=2;i<=numProteins;i++){u=C[cl][i];if(u=="_"||u=="NA")continue;
		M[u]=1;
		if(DRACONIAN) {
		    for(g in T)if(!(g in pGO[u]))delete T[g]
		} else {
		    for(g in T){T[g]=1;++numA[g]}
		}
	    }
	    #printf " col %d=%s, |T|=%d", i, u,length(T)
	}
	if(length(T)>0&&length(M)>1){
	    if(DRACONIAN) sum+=K_gset(T)
	    else for(g in T)if(numA[g]>1)sum+=numA[g]*K_g(g)/numProteins
	}
    }
    return sum
}
function K_C(C,
    sum,T){
    sum=0
    delete T; T[0]=1;delete T[0]; #make T the empty set
}

# Knowledge in a alignment of a bunch of clusters
function sim_A2(A){return K_A2(A)/length(GOfreq)}

#Old only-2-column alignment files
#ARGIND==1{C[$1]=$2; C_[$2]=$1}
#ARGIND==2&&($2 in C || $2 in C_){GOfreq[$3]++;pGO[$2][$3]=1}

BEGIN{DRACONIAN='$DRACONIAN'}
#Clusters version
ARGIND==1{for(i=1;i<=NF;i++){CA[FNR][i]=$i;pC[$i][FNR]=1}} # CA[][]=cluster alignment; pC[p] = clusters this protein is in.
ARGIND==2&&($2 in pC){GOfreq[$3]++;pGO[$2][$3]=1}
END{for(p in pC)if(!(p in pGO)){pGO[p][0]=1;delete pGO[p][0]} #proteins with no GO terms need pGO[p] explicit empty list.
    know=K_AC(CA); printf "%d %d %d %g %g\n", length(CA), length(pGO), length(GOfreq), know, know/length(GOfreq)
}' "$@"
