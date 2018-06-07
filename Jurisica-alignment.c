#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include "misc.h"
#include "sets.h"
#include "bintree.h"
#include "rand48.h"

#define MAX_GENES 24252
#define MAX_mRNAs 59530
#define MAX_SPECIES 535
#define MAX_HOLES 5000 // Homo_sapiens has about 4300 mRNAs so this should be plenty.
static char *gName[MAX_GENES], *rName[MAX_mRNAs], *speciesName[MAX_SPECIES];
static int numGenes, numRNAs, numSpecies, species2numRNAs[MAX_SPECIES];
static unsigned short int speciesRNAid[MAX_SPECIES][MAX_HOLES];
static BINTREE *gNameTree, *rNameTree;
static SET geneSet[MAX_mRNAs];

void ReadFile(FILE *fp, int speciesInt)
{
    char line[BUFSIZ], word1[BUFSIZ], word2[BUFSIZ];
    int lineNum = 0;
    if(!gNameTree) gNameTree = BinTreeAlloc(unbalanced, strcmp, strdup, free, NULL, NULL);
    if(!rNameTree) rNameTree = BinTreeAlloc(unbalanced, strcmp, strdup, free, NULL, NULL);

    while(fgets(line, sizeof(line), fp))
    {
	long idGene, idRNA; // THESE NEED TO BE LONG SINCE FOINT HAS SIZE 8
	assert(2 == sscanf(line, "%s\t%s\n", word1, word2));
	if(!BinTreeLookup(gNameTree, (foint)word1, &idGene))
	{
	    idGene = numGenes;
	    //fprintf(stderr, "GENE %d %s\n", numGenes, word1);
	    BinTreeInsert(gNameTree, (foint)word1, (foint)idGene);
	    gName[numGenes] = strdup(word1);
	    assert(numGenes <= MAX_GENES);
	    assert(BinTreeLookup(gNameTree, (foint)word1, &idGene));
	    assert(idGene == numGenes);
	    ++numGenes;
	}
	if(!BinTreeLookup(rNameTree, (foint)word2, &idRNA))
	{
	    idRNA = numRNAs;
	    //fprintf(stderr, "miRNA %d %s\n", numRNAs, word2);
	    BinTreeInsert(rNameTree, (foint)word2, (foint)idRNA);
	    rName[numRNAs] = strdup(word2);
	    assert(numRNAs <= MAX_mRNAs);
	    speciesRNAid[speciesInt][species2numRNAs[speciesInt]++] = idRNA;
	    assert(BinTreeLookup(rNameTree, (foint)word2, &idRNA));
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

// Our goal is to minimize the difference between connection sets across the alignment.
// To do this, we union all the connection sets together over each hole to get the full
// connection set above the hole, and then add up the XORs of each to that total; the
// sum of the XORs is the objective we're trying to minimize.
long ObjectiveOverHole(int hole)
{
    int level, numPegs = 0;
    long value = 0;
    SET *fullUnion = SetAlloc(MAX_GENES), *XOR = SetAlloc(MAX_GENES);
    for(level=0; level<numSpecies; level++)
	if(_alignment[hole][level])
	    SetUnion(fullUnion, fullUnion, _alignment[hole][level]);
    for(level=0; level<numSpecies; level++)
	if(_alignment[hole][level])
	{
	    ++numPegs;
	    value += SetCardinality(SetXOR(XOR, fullUnion, _alignment[hole][level]));
	}
    SetFree(fullUnion);
    SetFree(XOR);
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

static long int _currentObjective;

void HillClimbing(long iterations)
{
    long i, sameCount = 0;
    for(i=0; i<iterations;i++)
    {
	if(i % 10000000==0) fprintf(stderr, "i=%ld, Objective = %ld\n", i, _currentObjective);
	int peg1, peg2, level = numSpecies * drand48();
	if(species2numRNAs[level] < 2) continue;
	do {
	    peg1 =  drand48() * MAX_HOLES; //species2numRNAs[level];
	    peg2 =  drand48() * MAX_HOLES; //species2numRNAs[level];
	} while (peg1==peg2 || (!_alignment[peg1][level] && !_alignment[peg2][level]));
	SET *set1 = _alignment[peg1][level];
	SET *set2 = _alignment[peg2][level];
	long oldTower1 = ObjectiveOverHole(peg1);
	long oldTower2 = ObjectiveOverHole(peg2);
	// Now check the swap
	_alignment[peg1][level] = set2;
	_alignment[peg2][level] = set1;
	long newTower1 = ObjectiveOverHole(peg1);
	long newTower2 = ObjectiveOverHole(peg2);
	long change = (newTower1-oldTower1)+(newTower2-oldTower2);
	if(change < 0) // good move!
	{
	    sameCount = 0;
	    _currentObjective += change;
	}
	else //it's a BAD move so reset
	{
	    _alignment[peg1][level] = set1;
	    _alignment[peg2][level] = set2;
	    if(sameCount++>1000000) return;
	}
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
    for(level = 0; level < numSpecies; level++)
    {
	int nRNAs = species2numRNAs[level], permutation[MAX_HOLES];
	assert(nRNAs < MAX_HOLES);
	RandomPermutation(nRNAs, permutation);
	for(hole=0; hole < nRNAs; hole++)
	    _alignment[hole][level] = &geneSet[speciesRNAid[level][permutation[hole]]];
    }
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
