/*
** The incremental Objective could probably be even faster. Currently we need to recompute the objective value
** of two entire towers when we simply swap 2 pegs at one level of that tower. To do better we'd need to keep
** track of the setUnion incrementally by keeping track, at each bit position, how many species contribute to
** that unionBit.  When that count gets to zero, we can turn off that bit in the setUnion.  This is basically
** like a shadow network, except it'll be a shadowUnion.
*/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include "misc.h"
#include "sets.h"
#include "multisets.h"
#include "bintree.h"
#include "rand48.h"

#define MAX_GENES 24252
#define MAX_mRNAs 59530
#define MAX_SPECIES 535
#define MAX_HOLES 5000 //MAX_mRNAs // Homo_sapiens has about 4300 mRNAs but let's be pessimistic since we heavily penalize lonely pegs
static char *gName[MAX_GENES], *rName[MAX_mRNAs], *speciesName[MAX_SPECIES];
static int numGenes, numRNAs, numSpecies, species2numRNAs[MAX_SPECIES];
static unsigned short int speciesRNAid[MAX_SPECIES][MAX_HOLES];
static BINTREE *gNameTree, *rNameTree;
static SET geneSet[MAX_mRNAs];

void ReadFile(FILE *fp, int speciesInt)
{
    char line[BUFSIZ], word1[BUFSIZ], word2[BUFSIZ];
    int lineNum = 0;
    if(!gNameTree)gNameTree=BinTreeAlloc(unbalanced, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, NULL);
    if(!rNameTree)rNameTree=BinTreeAlloc(unbalanced, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, NULL);

    while(fgets(line, sizeof(line), fp))
    {
	long idGene, idRNA; // THESE NEED TO BE LONG (sizeof foint) since BinTreeLookup writes a foint into their addresses!
	// Could fix it by adding a parameter to BinTreeLookup which is sizeof the info we're looking up...
	assert(sizeof(foint) == sizeof(idRNA));
	assert(2 == sscanf(line, "%s\t%s\n", word1, word2));
	if(!BinTreeLookup(gNameTree, (foint)word1, (foint*)&idGene))
	{
	    idGene = numGenes;
	    //fprintf(stderr, "GENE %d %s\n", numGenes, word1);
	    BinTreeInsert(gNameTree, (foint)word1, (foint)idGene);
	    gName[numGenes] = strdup(word1);
	    assert(numGenes <= MAX_GENES);
	    assert(BinTreeLookup(gNameTree, (foint)word1, (foint*)&idGene));
	    assert(idGene == numGenes);
	    ++numGenes;
	}
	if(!BinTreeLookup(rNameTree, (foint)word2, (foint*)&idRNA))
	{
	    idRNA = numRNAs;
	    //fprintf(stderr, "miRNA %d %s\n", numRNAs, word2);
	    BinTreeInsert(rNameTree, (foint)word2, (foint)idRNA);
	    rName[numRNAs] = strdup(word2);
	    assert(numRNAs <= MAX_mRNAs);
	    speciesRNAid[speciesInt][species2numRNAs[speciesInt]++] = idRNA;
	    assert(BinTreeLookup(rNameTree, (foint)word2, (foint*)&idRNA));
	    assert(idRNA == numRNAs);
	    ++numRNAs;
	}
	SetAdd(&geneSet[idRNA], idGene);

	lineNum++;
    }
}

// Since C is row-major, and a move consists of moving a peg from one hole to another,
// we want the ability to make a temporary copy of the tower above a hole.  To preserve
// locality of reference, we store the tower above the hole in a row of the matrix even
// though we *think* of it as a column.
static SET *_alignment[MAX_HOLES][MAX_SPECIES];
static MULTISET *_shadowUnion[MAX_HOLES];

/* _numDisagree tells us, for each of the genes (ie edges emating from pegs
** above a certain hole), how many total pegs disagree with the _shadowUnion
** for that gene.  This allows us to incrementally update the objective
** quickly when one of the elements of _shadowUnion reaches zero or jumps up
** from zero to nonzero: essentially every element in every peg over that
** hole that previously disagreed, now agrees, and vice versa.  If we have
** N pegs over a hole and at each element e we know that D[e] of them disagreed,
** then if the shadowUnion flips at edge e then the count D[e] should swap
** to D[e]=N-D[e]; let's call that the complement of D[e] with respect to N.
** For example, say 5 pegs agree to connect to exactly one gene
** G but a 6th peg connects to G plus to 4 other genes. (You can picture this
** by holding your left hand vertically; the 5 fingers are each single edges emanating
** from 5 pegs; they line up on top of each other
** pointing at the same gene; your right hand held horizontally splayed out with the
** middle finger above the left hand.  5Left + middle = 6 pointing at gene G; 4 other
** fingers pointing at other genes.) In that case the
** shadowUnion will have 5 elements, 4 of them with multiplicity 1 and one
** of them with multiplcity 6; in that case N=6 pegs; the element with multiplcity
** 6 will have _numDisagree equal to 0 but the ones with multiplicity 1 will
** all have numDisagree equal to 5 since 5 pegs disagree with the union
** at that element.  If we remove the outlier peg, then there are 5 pegs
** left (newPegCount=5), all agree with each other; the 4 elements that had
** multiplicity 1 (all with D[e]=5) will disappear, leaving multiplicity
** D[e]=newPegCount-D[e]=0. This allows us to incrementally update the objective
** quickly. In fact the same trick may help us to compute CIQ quickly in SANA.
*/
static MULTISET *_numDisagree[MAX_HOLES];
static FREQTYPE _numPegs[MAX_HOLES]; // FREQTYPE since that's what MULTISETS can store.
static long _shadowObjective[MAX_HOLES];

// Our goal is to minimize the difference between connection sets across the alignment.
// To do this, we union all the connection sets together over each hole to get the full
// connection set above the hole, and then add up the XORs of each to that total; the
// sum of the XORs is the objective we're trying to minimize.
SET *SetUnionOverHole(SET *setUnion, int hole)
{
    int level;
    if(setUnion)
    {
	assert(setUnion->n == MAX_GENES);
	SetEmpty(setUnion);
    }
    else
	setUnion = SetAlloc(MAX_GENES);
    for(level=0; level<numSpecies; level++)
	if(_alignment[hole][level])
	    SetUnion(setUnion, setUnion, _alignment[hole][level]);
    return setUnion;
}

long ObjectiveAtHoleLevel(SET *setUnion, int hole, int level)
{
    static SET *XOR;
    if(!XOR) XOR = SetAlloc(MAX_GENES);
    return SetCardinality(SetXOR(XOR, setUnion, _alignment[hole][level]));
}

long ObjectiveOverHole(int hole)
{
    int level, numPegs = 0;
    long value = 0;
    SET *setUnion = SetAlloc(MAX_GENES), *XOR = SetAlloc(MAX_GENES), *tmp=SetAlloc(MAX_GENES);
    SetUnionOverHole(setUnion, hole);
    assert(SetEq(MultisetToSet(tmp, _shadowUnion[hole]), setUnion));
    for(level=0; level<numSpecies; level++)
	if(_alignment[hole][level])
	{
	    ++numPegs;
	    long tmp1 = ObjectiveAtHoleLevel(setUnion, hole, level);
	    long tmp2 = SetCardinality(SetXOR(XOR, setUnion, _alignment[hole][level]));
	    assert(tmp1==tmp2);
	    value += tmp2;
	}
    SetFree(setUnion);
    SetFree(XOR);
    SetFree(tmp);
    assert(numPegs = _numPegs[hole]);
    if(numPegs == 1) value += numGenes; // make it really unsavoury to be a lonely peg.
    return value;
}

long Objective(void)
{
    long result = 0;
    int hole;
    for(hole=0; hole<MAX_HOLES; hole++)
	result += ObjectiveOverHole(hole);
    return result;
}

static long int _currentObjective, _currentFastObjective;

void HillClimbing(long iterations)
{
    long i, sameCount = 0;
    SET *setUnion1 = SetAlloc(MAX_GENES), *setUnion2 = SetAlloc(MAX_GENES), *XOR = SetAlloc(MAX_GENES);
    SET *setUnion = SetAlloc(MAX_GENES);
    for(i=0; i<iterations;i++)
    {
	if(i % 10000==0) fprintf(stderr, "i=%ld, Objective = %ld\n", i, _currentObjective);
	int peg1, peg2, level = numSpecies * drand48();
	// if(species2numRNAs[level] < 2) continue; // No, let the one mRNA move around.
	do {
	    peg1 =  drand48() * MAX_HOLES; //species2numRNAs[level];
	    peg2 =  drand48() * MAX_HOLES; //species2numRNAs[level];
	} while (peg1==peg2 || (!_alignment[peg1][level] && !_alignment[peg2][level]));
	SET *set1 = _alignment[peg1][level];
	SET *set2 = _alignment[peg2][level];
	long oldTower1 = ObjectiveOverHole(peg1);
	long oldTower2 = ObjectiveOverHole(peg2);

	MultisetToSet(setUnion1, _shadowUnion[peg1]);
	MultisetToSet(setUnion2, _shadowUnion[peg2]);
	SetUnionOverHole(setUnion, peg1);
	assert(SetEq(setUnion, setUnion1));
	SetUnionOverHole(setUnion, peg2);
	assert(SetEq(setUnion, setUnion2));
#if 0
	assert(oldTower1 == _shadowObjective[peg1]);
	assert(oldTower2 == _shadowObjective[peg2]);
	long oldShadow1 = SetCardinality(SetXOR(XOR, setUnion1, set1));
	long oldShadow2 = SetCardinality(SetXOR(XOR, setUnion2, set2));
	MultisetDeleteSet(_shadowUnion[peg1], set1);
	MultisetDeleteSet(_shadowUnion[peg2], set2);
	MultisetAddSet(_shadowUnion[peg1], set2);
	MultisetAddSet(_shadowUnion[peg2], set1);
	MultisetToSet(setUnion1, _shadowUnion[peg1]);
	MultisetToSet(setUnion2, _shadowUnion[peg2]);
	long newShadow1 = SetCardinality(SetXOR(XOR, setUnion1, set2));
	long newShadow2 = SetCardinality(SetXOR(XOR, setUnion2, set1));
	long fastChange = newShadow1+newShadow2 - (oldShadow1+oldShadow2);

	MultisetToSet(setUnion1, _shadowUnion[peg1]);
	MultisetToSet(setUnion2, _shadowUnion[peg2]);
	long newShadow1 = ObjectiveAtHoleLevel(setUnion1, peg1, level);
	long newShadow2 = ObjectiveAtHoleLevel(setUnion2, peg2, level);
	long fastChange = (newShadow1-oldShadow1)+(newShadow2-oldShadow2);
#endif
	// Now check the swap
	MultisetDeleteSet(_shadowUnion[peg1], set1);
	MultisetDeleteSet(_shadowUnion[peg2], set2);
	MultisetAddSet(_shadowUnion[peg1], set2);
	MultisetAddSet(_shadowUnion[peg2], set1);
	_alignment[peg1][level] = set2;
	_alignment[peg2][level] = set1;
	long newTower1 = ObjectiveOverHole(peg1);
	long newTower2 = ObjectiveOverHole(peg2);

	long change = (newTower1-oldTower1)+(newTower2-oldTower2);
	//assert(fastChange == change);

	if(change <= 0) // good move!
	{
	    if(change < 0) sameCount = 0;
	    _currentObjective += change;
	    //_currentFastObjective += fastChange;
	}
	else //it's a BAD move so reset
	{
#if 1
	    MultisetDeleteSet(_shadowUnion[peg2], set1);
	    MultisetDeleteSet(_shadowUnion[peg1], set2);
	    MultisetAddSet(_shadowUnion[peg2], set2);
	    MultisetAddSet(_shadowUnion[peg1], set1);
#endif
	    _alignment[peg1][level] = set1;
	    _alignment[peg2][level] = set2;
	    if(sameCount++>1000000) return;
	}
	//assert(_currentFastObjective == _currentObjective);
	//assert(_currentObjective == Objective());
	//printf("%ld (%ld -> %ld)", i, oldVal, newVal); fflush(stdout);
    }
}

// permutation array is nuked and replaced with a random permutation of the integers 0 through n-1 inclusive.
void RandomPermutation(int n, int *permutation)
{
    int i;
    for(i=0; i<n; i++) permutation[i] = i;
    for(i=0; i<n; i++) 
    {
	int position = (n-i)*drand48();
	int tmp = permutation[i];
	permutation[i] = permutation[i+position];
	permutation[i+position] = tmp;
    }
}

void PrintAlignment(void)
{
    int level, hole;
    for(hole = 0; hole < MAX_HOLES; hole++)
    {
	Boolean print = false;
	char line[BUFSIZ];
	line[0] = '\0';
	for(level=0; level < numSpecies; level++)
	{
	    int mRNAid = -1;
	    SET *mRNAset = _alignment[hole][level];
	    if(mRNAset)
	    {
		mRNAid = mRNAset - geneSet; // pointer arithmetic.
		assert(mRNAid >= 0 && mRNAid < numRNAs);
		if(strlen(rName[mRNAid]) == 0) Fatal("Hmm, mRNAid %d has zero-length name", mRNAid);
		strcat(line, rName[mRNAid]);
		print = true;
	    }
	    else
		strcat(line,"_");
	    if(level < numSpecies-1) strcat(line, "\t");
	}
	if(print) printf("%s\n", line);
    }
}

void InitializeAlignment(void)
{
    int level, hole;
    for(hole=0; hole < MAX_HOLES; hole++)
    {
	_shadowUnion[hole] = MultisetAlloc(MAX_GENES);
	assert(_shadowUnion[hole]);
    }

    for(level = 0; level < numSpecies; level++)
    {
	int nRNAs = species2numRNAs[level], permutation[MAX_HOLES];
	assert(nRNAs < MAX_HOLES);
	RandomPermutation(nRNAs, permutation);
	for(hole=0; hole < nRNAs; hole++)
	{
	    MultisetAddSet(_shadowUnion[hole],
		(_alignment[hole][level] = &geneSet[speciesRNAid[level][permutation[hole]]])
	    );
	}
    }

    SET *setUnion = SetAlloc(MAX_GENES), *tmp = SetAlloc(MAX_GENES);
    _currentFastObjective = 0;
    for(hole=0; hole < MAX_HOLES; hole++)
    {
	SetUnionOverHole(setUnion, hole);
	MultisetToSet(tmp, _shadowUnion[hole]);
	assert(SetEq(setUnion, tmp));
	//_currentFastObjective += (_shadowObjective[hole] = ObjectiveOverHole(hole));
    }
    SetFree(setUnion);
    SetFree(tmp);
}

int main(int argc, char *argv[])
{
    srand48(time(NULL)+getpid());
    int argn = 1, i;
    fprintf(stderr,"Initializing mRNA sets...");
    for(i=0; i<MAX_mRNAs; i++) SetResize(&geneSet[i],MAX_GENES);
    
    if(argn == argc)
	Fatal("Need some files!");
    else
    {
	fprintf(stderr, "reading files...");
	while(argn < argc)
	{
	    FILE *in;
	    if( (in = fopen(argv[argn], "r")) == NULL)
		perror(argv[argn]);
	    else
	    {
		//fprintf(stderr, "FILENAME %d (%s)\n", argn, argv[argn]);
		assert(strstr(argv[argn],".el"));
		int len=strlen(argv[argn]);
		assert(argv[argn][len-3]=='.' && argv[argn][len-2]=='e' && argv[argn][len-1]=='l');
		argv[argn][len-3]='\0';
		speciesName[numSpecies] = strdup(argv[argn]);
		ReadFile(in, numSpecies);
		++numSpecies;
		fclose(in);
	    }
	    ++argn;
	}
    }
    //for(i=0; i<numRNAs; i++) fprintf(stderr, "CARDINALITY %d %d\n", i, SetCardinality(&geneSet[i]));
#if 0
    SET *tmp = SetAlloc(MAX_GENES);
#define THREE_COLUMN 0 // set to zero to get the 60,000 x 60,000 ASCII matrix

    for(i=0; i<numRNAs; i++)
    {
#if THREE_COLUMN
	for(j=i+1; j<numRNAs; j++)
	    printf("%d\t%d\t%d\n", i, j, SetCardinality(SetXOR(tmp, &geneSet[i], &geneSet[j])));
#else
	printf("%d", SetCardinality(SetXOR(tmp, &geneSet[i], &geneSet[0])));
	for(j=1; j<numRNAs; j++)
	    printf("\t%d", SetCardinality(SetXOR(tmp, &geneSet[i], &geneSet[j])));
	printf("\n");
#endif
    }
#endif
    fprintf(stderr, "Intial Random Alignment...\n");fflush(stderr);
    InitializeAlignment();
    _currentObjective = Objective();
    double startTime=uTime();
    printf("Objective Function: Initial %ld, ", _currentObjective); fflush(stdout);
    HillClimbing(100000000);
    printf("Final %ld, time = %g\n", _currentObjective, uTime()-startTime);
    PrintAlignment();
    return 0;
}
