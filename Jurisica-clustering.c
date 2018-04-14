/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "misc.h"
#include "sets.h"

#define NUM_GENES 24252
#define NUM_mRNAs 59530
static char *gName[NUM_GENES], *rName[NUM_mRNAs];
static int numGenes, numRNAs;

void ReadFile(FILE *fp)
{
    char line[BUFSIZ], word1[BUFSIZ], word2[BUFSIZ];
    int lineNum = 0;

    while(fgets(line, sizeof(line), fp))
    {
	int i;
	assert(2 == sscanf(line, "%s %s", word1, word2));
	for(i=0; i < numGenes; i++)
	    if(strcmp(gName[i], word1) == 0)
		/* add it to the set */;
	if(i == numGenes)
	    gName[numGenes++] = strdup(word1);
	assert(numGenes <= NUM_GENES);

	for(i=0; i < numRNAs; i++)
	    if(strcmp(rName[i], word2) == 0)
		/* add it to the set */;
	if(i == numRNAs)
	    rName[numRNAs++] = strdup(word2);
	assert(numRNAs <= NUM_mRNAs);

	lineNum++;
    }
}

int main(int argc, char *argv[])
{
    int argn = 1;
    
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
		ReadFile(in);
		fclose(in);
	    }
	    ++argn;
	}
    }
    return 0;
}
