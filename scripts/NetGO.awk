#!/bin/sh
# Sept. 2019
# NetGO.awk (c) Wayne B. Hayes (whayes@uci.edu). 
# Yes, we call this NetGO.awk even though technically it's a Bourne Shell script. Most of the "meat" of the implementation
# is in the awk at the bottom of this Bourne shell script.

USAGE="USAGE: $0 [-L] gene2goFile alignFile[s]
    -L: 'Lenient'. The default behavior is what we call 'Dracanion' in the paper, which
    insists that a GO term must annotate every protein in a cluster for it to count.
    The Lenient option gives a GO term a weight per-cluster that is scaled by the
    number of proteins it annotates (so long as it's more than 1).

    alignFile: each line consists of a cluster of any number of proteins; proteins can
    appear in more than one cluster. Note it must use the same protein naming convention
    as your gene2go file.

    gene2goFile: a standard-format gene2go file downloaded from the GO consortium's
    website. For now we only use columns 2 (protein name) and 3 (GO term). We ignore
    the species, which is a simplification since about 20% of proteins appear in two
    species, but only 2% appear in more than 2.
"
die() { echo "$USAGE" >&2; echo "$@" >&2; exit 1
}

# References:
# Main paper this script is based on is the shorter 2019 paper by Hayes, "NetGO: a data-driven approach..."
# Older paper with more justification: Hayes and Mamano 2017, doi: 10.1093/bioinformatics/btx716 "SANA NetGO: a combinatorial approach..."

# IMPLEMENTATION DECISIONS
# Though the math for NetGO is pretty straightforward, there are are a large number of implementation decisions that must be made.
# Few of these are carved in stone.
#
# First, for now WE IGNORE SPECIES IN THE gene2go FILE.
# Most proteins (about 80%) occur in only one species, so only 20% occur in more than one species, and only about 2% occur in more
# than two species. Plus, it is a good bet (though not certain) that the *exact* same protein will perform a highly similar set of
# functions in whatever species it appears in. So GO terms valid for one species are likely to be valid for the same protein in
# another species.
# Furthermore, the species-taxonomic ID (which is the first columns of the gene2go file) is not unique. For example, at last count
# S.cerevisiae has 321 (!!! yes over 300) different strains, and thus over 300 taxonomic IDs. Even mouse had over a dozen different
# taxonomic IDs. Dealing with this complexity would be a mess.
# For the above reasons, for now we ignore the first column (taxonomic ID) of the gene2go file, and assume than the pair (gene,GO)
# is enough to determine GO counts, GO frequencies, and protein annotations.

# Second, protein names are always in flux, and some proteins have vastly more annotations than others. Even if you use the same naming
# convention in your alignment file as the appears in the gene2go file (which is necessary for this script to work), there will likely
# be some proteins in your alignment that do not appear in the gene2go file, and some proteins in the gene2go file for your species
# that are not in your alignment file. Finally, the gene2go file is HUGE, and we don't want to read the whole thing in every time.
# Thus, this script needs to know YOUR list of proteins (gotten from your alignment/cluster file) before it reads the gene2go file;
# it will only record annotation information for the proteins you need.

# The differing sets of proteins (your alignment file vs. those in the gene2go file) also brings up the question of "how many
# proteins", and "how many GO terms", are there, when we need to know their count? The GO term frequency (kappa_g in the paper
# is based on how many times we see the GO term g *among the proteins in your alignment file*---and recall that we also ignore
# species ID, so this count can fluctuate a bit depening on your input data.

# CLUSTER SIZE
# How many proteins are in your cluster? This depends on how we count. If the exact same protein name appears twice in the same
# cluster, does it count for 1, or 2? That is, do we remove duplicates? This may depend on what you want to do. Certainly
# seeing the same protein $p$ a second does not add to the set of GO annotations in this cluster after we've seen $p$ before,
# it should it change the *count* of these GO terms? 
# Things to consider in this question: though you may be tempted to remove duplicate proteins, this means you don't get any
# points for correctly putting both copies of the same protein into the same cluster *if they came from different species*; this
# is a significant drawback because correctly aligning a protein to itself between species is a pretty beneficial thing to do,
# and getting no points for it sucks. Furthermore, we claim in the paper that aligning a network to itself should, of course,
# result in a NetGO score (very close to) 1; if you remove duplicates then aligning a network to itself results in every cluster
# having only one protein, and a NetGO score of exactly zero.
# For these reasons, we have decided to allow duplicates. The number of times a protein $u$ appears in a cluster is kept in
# the variable M[u] (M=membership set).


# CODING CONVENTIONS
# AWK is an error prone language. I like it primarily because it's eminantly portable (should run anywhere a Linux distribution exists),
# it's lightning fast (usually faster than Java or Python), and it can be used on the Unix command line to do simple (or complex) tasks.
# Variables have dynamic type, and there are only 3 types: double precision, string, and array. All arrays are associative
# (ie., dictionaries), and an element of an array can be another array, and different elements of the same array can have different type.
#
# To implement "sets" in AWK, I use A[x]=1 to mean that x is in the set A; x can be a number, string, etc.
# Sometimes I interpret it as a multi-set, where the value is an integer rather than Boolean. such as the membership array
# M[] which contains string elements that are names of proteins that occur in the cluster currently being processed.
# (The multi-set is how we keep track of multiple occurances of the same protein in the same cluster.)
# Note also that in AWK, if a variable (even an array element) doesn't exist and you try to access it, it will be created
# and then returned with value "" (empty string) or zero, depending on context. THIS IS DANGEROUS, because it means you should
# not check for the existence of a member in an array by asking for its value, because each one you ask for, if it does not
# exist, creates a new (useless) member of the array. This can quickly cause the memory usage of awk to baloon. Instead,
# use the syntax (x in A) to check if A[x] exists, befory trying to access it. The (x in A) syntax does not create A[x] if
# it doesn't already exist.

# GLOBAL VARIABLES:
# By default all variables in AWK are global. The only exceptions are function parameters (see below).
# The primary data variables (global) in this program are set in the ARGIND program blocks at the bottom of the AWK script:
#
# CA is the "cluster alignment". Contents: CA[line_number][column_number]=proteinName
# That is, CA[L] is the cluster of proteins that occured on line L of the input cluster (align) file. The number of protein names,
# including duplicates, that occured on that line is length(CA[line_number]).
#
# pC is the protein-to-cluster multi-set, that tells us which cluster(s) a protein appeared in, and its multiplicity in that cluster
# Contains: pC[p][L]=count means protein p occurs in cluster (line number) L, count times.
# length(pC) is thus the number of unique protein names that occured in your alignment file;
# length(pC[p]) is the number of clusters # it appears in.
#
# pGO: the protein-to-GO set map
# Contains:  pGO[p][g]=1 if protein p has ever been annotated by GO term g.
# REMEMBER SPECIES ID IS IGNORED in the gene2go file. This means that there's a possibility that p has been annotated with g in
# another species, but not the one you're aligning. This will be (a) rare, and (b) probably not an entirely invalid assumption.
# Note: conceivably we could make this a count, as we do for GOp below. For now we don't.
#
# GOp: GO-to-protein multiset (inverse relationship of the above, but a multiset instead of just a set)
# Contains: GOp[g][p]=count, is the count of the number of times GO term g has annotated protein p. This usually means that
# the GO term occurs with protein p in the gene2go file many times, each with a different evidence code. This is useful information
# to have (eg., more evidence codes makes the annotation more certain to be correct), but for now we ignore the count and treat it
# simply as a set (true-or-false: does g annotate p?)
#
# EMPTY SETS
# In several places, we assume that the length of an array defines how many relationships it has. The problem is that,
# since arrays are actually associative, if the relationship does not exist at all, then the element will be not be zero, but
# will instead not exist. This breaks some parts of the code. To fix it, we sometimes create an empty array, to specify
# "this relationship does not exist". Unfortunately there is no way in AWK to create an empty array. The only way to do it
# is to bring the array into existing by accessing a (as-yet non-existant) member; both both creates the array, and the one
# new member. Then you can delete that one member, leaving an empty array. The syntax to do this (seen below in several places) is:
# A[0]=1; delete A[0]; # create dummy element A[0], then delete it, leaving A as an array with zero elements.

# awk allows no local variables other than parameters, but it also allows you to declare more
# parameters than are passed. Thus the convention is to declare local variables as extra parameters, as the second
# line of the parameter declarations.

DRACONIAN=1
case "$1" in
-L*) DRACONIAN=0;shift;;
-*) die "unknown option '$1'";;
esac
[ $# -ge 2 ] || die "expecting at least 2 arguments, alignFile and gene2goFile"

GENE2GO=$1; shift

for i
do
    hawk '
    # res1 is your variable, where the output set goes; it will be nuked and replaced with the set intersection of T1 and T2.
    function SetIntersect(res1,T1,T2,
	g){delete res1;if(length(T1)<length(T2)){for(g in T1)if(g in T2)res1[g]=1}
							   else{for(g in T2)if(g in T1)res1[g]=1}}
    # same as above but for set union, and res2 is the result.
    function SetUnion(res2,T1,T2,
	g){delete res2;for(g in T1)res2[g]=1;for(g in T2)res2[g]=1}
    # cumulative add set T to res3
    function SetCumulativeUnion(res3,T, g){for(g in T)res3[g]=1}

    # Return the "knowledge" (ie., specificity) of a single GO term g.
    function K_g(g){if(g in GOp) return 1/length(GOp[g]); else return 0}

    # Return the sum of specifities of a set of GO terms.
    function K_gset(T,
	g,sum){sum=0;for(g in T)sum+=K_g(g); return sum}

    # Return the "knowledge" level of a protein p, which is just K_gset(T), where T is its sets of GO annotations.
    function K_p(p,
	sum){sum=0;if(p in pGO)return K_gset(pGO[p]); else return 0}

    # Knowledge in a simple pairwise alignment in which A[u]=v
    # This function was mostly just used for testing simple cases early on; it is superceded by K_AC below.
    function K_A2(A,
	sum,u,v,Tuv){sum=0;for(u in A)if(u in pGO){v=A[u];if(v in pGO){SetIntersect(Tuv,pGO[u],pGO[v]); sum+=K_gset(Tuv)}}; return sum}

    # Knowledge in a general alignment of protein clusters
    # Note technically it would be nice to have a K_C function (knowledge inside a cluster) but AWK makes this hard, so
    # instead we just compute K_C inline, inside this function.
    function K_AC(C,
	sum,numClusterFields,cl,i,u,g,T,M,K_C){
	sum=0;
	for(cl in C){
	    K_C=0;
	    #printf "Cluster %d, length %d;",cl,length(C[cl])
	    numClusterFields=length(C[cl]);
	    delete M; M[0]=1;delete M[0]; # initialize M to empty list; M=set of protein members;
	    delete T; T[0]=1;delete T[0]; # initialize T to empty list; T=set of GO terms across the cluster;
	    if(numClusterFields>1){ # have to match more than one protein to be interesting (handle duplicates below)
		# initialize T to the GO terms of first protein in the cluster:
		# This initialization is necessary in order to incrementelly create the set intersection of GO terms below.
		u=C[cl][1]; if(u!="_"&&u!="NA"){ # _ and NA usually replesent dummy place-holders in alignment files
		    ++M[u] # multi-set: keep track if protein u occurs more than once.
		    for(g in pGO[u])++T[g];
		}
		# Now go through the rest of the cluster, removing elements from T that do not occur in subsequent columns (DRACONIAN)
		# and incremently updating the count T[g] if the GO term annotates more than one protein.
		for(i=2;i<=numClusterFields;i++){u=C[cl][i];if(u=="_"||u=="NA")continue;
		    ++M[u];
		    if(DRACONIAN) {
			for(g in T)if(!(g in pGO[u]))delete T[g] # cumulatively create the set intersection of common GO terms
		    } else {
			for(g in T)++T[g];
		    }
		}
		#printf " col %d=%s, |T|=%d", i, u,length(T)
	    }
	    if(length(T)>0 && # if this cluster has any annotations...
		(length(M)>1 || # and it has more than one protein...
		    (length(M)==1 && M[u]>1))) # or the same protein multiple times...
	    {
		if(DRACONIAN) K_C += K_gset(T)
		else {
		    #for(i in M)numClusterFields-=(M[i]-1)
		    for(g in T)if(T[g]>1) # g must annotate more than 1 protein to get any "points"
			K_C += T[g]*K_g(g)/numClusterFields
		}
	    }
	    #printf " K=%g\n",K_C
	    sum+=K_C
	}
	return sum
    }

    # Knowledge in a alignment of a bunch of clusters
    function sim_A2(A){return K_A2(A)/length(GOp)}

    #Old 2-column alignment files, obsolete but simple to test
    #ARGIND==1{C[$1]=$2; C_[$2]=$1}
    #ARGIND==2&&($2 in C || $2 in C_){pGO[$2][$3]=1;++GOp[$3][$2]}

    BEGIN{DRACONIAN='$DRACONIAN'}
    #Clusters version
    ARGIND==1{for(i=1;i<=NF;i++){CA[FNR][i]=$i;++pC[$i][FNR]}} # CA[][]=cluster alignment; pC[p] = clusters this protein is in.
    ARGIND==2&&($2 in pC){pGO[$2][$3]=1;++GOp[$3][$2]}
    END{
	if(length(pGO) < 0.001 * length(pC)) {
	    print "Warning: Fewer than 0.1% of the proteins in your alignment have GO annotations. This typically happens\n" \
		"when your alignment files use a different gene/protein naming convention than the gene2go file." >"/dev/fd/2"
	}
	for(p in pC)if(!(p in pGO)){pGO[p][0]=1;delete pGO[p][0]} #proteins with no GO terms need pGO[p] explicit empty list.
	know=K_AC(CA);
	printf "%s: Asize %d numP %d numGO %d K %g score %g\n", ARGV[1], length(CA), length(pGO), length(GOp), know, know/length(GOp)
    }' "$i" $GENE2GO
done
