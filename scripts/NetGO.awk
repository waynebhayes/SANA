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
function SetEmpty(S){delete S; S[0]=1;delete S[0]}
function SetIntersect(result,T1,T2,
    g){delete result;if(length(T1)<length(T2)){for(g in T1)if(g in T2)result[g]=1}
                                                       else{for(g in T2)if(g in T1)result[g]=1}}
function SetUnion(result,T1,T2,
    g){delete result;for(g in T1)result[g]=1;for(g in T2)result[g]=1}
function SetCumulativeUnion(result,T1, g){for(g in T1)result[g]=1}

function K_g(g){if(g in GOfreq) return 1/GOfreq[g]; else return 0}

function K_gset(T,
    g,sum){sum=0;for(g in T)sum+=K_g(g); return sum}

function K_p(p,
    sum){sum=0;if(p in pGO)return K_gset(pGO[p]); else return 0}

# Knowledge in a pairwise alignment in which A[u]=v
function K_A2(A,
    sum,u,v,Tuv){sum=0;for(u in A)if(u in pGO){v=A[u];if(v in pGO){SetIntersect(Tuv,pGO[u],pGO[v]); sum+=K_gset(Tuv)}}; return sum}

function sim_A2(A){return K_A2(A)/length(GOfreq)}

ARGIND==1{A[$1]=$2; A_[$2]=$1}
ARGIND==2&&($2 in A || $2 in A_){GOfreq[$3]++;pGO[$2][$3]=1}
END{ know=K_A2(A); printf "%d %d %d %g %g\n", length(A), length(pGO), length(GOfreq), know, know/length(GOfreq)}' "$@"
