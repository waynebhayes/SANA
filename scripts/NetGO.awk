#!/bin/sh
USAGE="$0 alignFile gene2go"
# Note awk cannot return arrays, so anything expecting an array as output must give that array as the first argument,
# which is passed by reference, and which will contain the result.
# awk global variables:
# pGO: an array pGO[proteinName string][GOname string]=1 if protein p (string) is annotated by GO term g (string).
# GOfreq[GOterm g]: the (global) frequency of GO term g.

# Coding convention: awk allows no local variables other than parameters, but it also allows you to declare more
# parameters than are passed. Thus the convention is to declare local variables as extra parameters, as the second
# line of the parameter declarations.

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

# Knowledge in a pairwise alignment in which A[u]=v
function K_A2(A,
    sum,u,v,Tuv){sum=0;for(u in A)if(u in pGO){v=A[u];if(v in pGO){SetIntersect(Tuv,pGO[u],pGO[v]); sum+=K_gset(Tuv)}}; return sum}

# Knowledge in a pairwise alignment in which A[u]=v
function K_C_Draconian(C,
    sum,cl_len,cl,i,u,g,T,M){
    sum=0;
    for(cl in C){
	#printf "Cluster %d, length %d;",cl,length(C[cl])
	cl_len=length(C[cl]);
	delete M; M[0]=1;delete M[0]; # initialize M to explicit empty list.
	delete T; T[0]=1;delete T[0]; # initialize T to explicit empty list.
	if(cl_len>1){ # have to match more than one protein to be interesting.
	    # initialize T to the GO terms of first protein in the cluster:
	    u=C[cl][1]; if(u!="_"&&u!="NA"){M[u]=1;for(g in pGO[u])T[g]=1}
	    for(i=2;i<=cl_len;i++){u=C[cl][i];if(u=="_"||u=="NA")continue;
		M[u]=1;
		for(g in T)if(!(g in pGO[u]))delete T[g]
	    }
	    #printf " col %d=%s, |T|=%d", i, u,length(T)
	}
	if(length(T)>0&&length(M)>0)sum+=K_gset(T)
    }
    return sum
}

# Knowledge in a alignment of a bunch of clusters
function sim_A2(A){return K_A2(A)/length(GOfreq)}

#Old only-2-column alignment files
#ARGIND==1{C[$1]=$2; C_[$2]=$1}
#ARGIND==2&&($2 in C || $2 in C_){GOfreq[$3]++;pGO[$2][$3]=1}

#Clusters version
ARGIND==1{for(i=1;i<=NF;i++){C[FNR][i]=$i;pC[$i][FNR]=1}} # C[][]=cluster map; pC[p] = clusters this protein is in.
ARGIND==2&&($2 in pC){GOfreq[$3]++;pGO[$2][$3]=1}
END{for(p in pC)if(!(p in pGO)){pGO[p][0]=1;delete pGO[p][0]} #proteins with no GO terms need pGO[p] explicit empty list.
    know=K_C_Draconian(C); printf "%d %d %d %g %g\n", length(C), length(pGO), length(GOfreq), know, know/length(GOfreq)
}' "$@"
