/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "misc.h"
#include "sets.h"
#include "bintree.h"

#define MAX_GENES 25000 // 24252
#define MAX_mRNAs 60000 // 59530
static char *gName[MAX_GENES], *rName[MAX_mRNAs];
static int numGenes, numRNAs;
static BINTREE *gNameTree, *rNameTree;
static SET *myGenes[MAX_mRNAs];

void ReadFile(FILE *fp)
{
    char line[BUFSIZ], word1[BUFSIZ], word2[BUFSIZ];
    int lineNum = 0;
    if(!gNameTree) gNameTree = BinTreeAlloc(unbalanced, strcmp, strdup, free, NULL, NULL);
    if(!rNameTree) rNameTree = BinTreeAlloc(unbalanced, strcmp, strdup, free, NULL, NULL);

    while(fgets(line, sizeof(line), fp))
    {
	int idGene, idRNA;
	assert(2 == sscanf(line, "%s\t%s\n", word1, word2));
	if(!BinTreeLookup(gNameTree, (foint)word1, &idGene))
	{
	    idGene = numGenes;
	    BinTreeInsert(gNameTree, (foint)word1, (foint)numGenes);
	    gName[numGenes++] = strdup(word1);
	    //printf("%d %s\n", numGenes, word1);
	    assert(numGenes <= MAX_GENES);
	}
	if(!BinTreeLookup(rNameTree, (foint)word2, &idRNA))
	{
	    idRNA = numRNAs;
	    BinTreeInsert(rNameTree, (foint)word2, (foint)numRNAs);
	    rName[numRNAs++] = strdup(word2);
	    //printf("%d %s\n", numRNAs, word2);
	    assert(numRNAs <= MAX_mRNAs);
	}
	SetAdd(myGenes[idRNA], idGene);

	lineNum++;
    }
}

int main(int argc, char *argv[])
{
    int argn = 1, i, j;

    for(i=0; i<MAX_mRNAs; i++) myGenes[i] = SetAlloc(MAX_GENES);
    
    if(argn == argc)
	ReadFile(stdin);
    else
    {
	while(argn < argc)
	{
	    FILE *in;
	    if( (in = fopen(argv[argn], "r")) == NULL)
		perror(argv[argn]);
	    else
	    {
		printf("FILENAME %d (%s)\n", argn, argv[argn]);
		ReadFile(in);
		fclose(in);
	    }
	    ++argn;
	}
    }

    SET *tmp = SetAlloc(MAX_GENES);
    for(i=0; i<numRNAs; i++) for(j=i+1; j<numRNAs; j++)
    {
	printf("%s\t%s\t%d\n", rName[i], rName[j], SetCardinality(SetXOR(tmp, myGenes[i],myGenes[j])));
    }
    return 0;
}
