#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
** This file is intended to take the data/*.txt files and create binary
** versions in whatever format your local machine uses; these binary versions
** then allow BLANT/faye to work much faster at sampling.  NOTE however
** that if we ever use a k greater than 8 (ie., graphlets larger than 8)
** then this code will need to be rewritten.  Right now it assumes the
** very nice, compact representation that graphlets of maximum 8 nodes
** allows: exactly 3 bits required per node to specify the permutation,
** times 8 nodes, is exactly 24 bits to specify a full permutation from a
** non-canonical to a canonical.  We also use this representation for k<8,
** even though it's not as compact as possible, it's good enough.  9 bits
** will make it more messy: you'd need 4 bits to specify a permutation,
** times 9 nodes gives 36 bits, which requires 5 bytes (4.5 actually)
** which doesn't even fit into a 32-bit int.  We'd need to move to 64-bit
** ints for the intermediate representation (Would be more compact to put
** two permutations into 9 bytes, but I digress.)  10 nodes would be nice
** too, then it would be 10*4=40 bits or exactly 5 bytes.  Technically we
** could go all the way to 16 node graphlets fitting into a 64-bit int or
** 8 bytes, but that would probably require more RAM than would fit into
** the known universe.
*/

#ifndef k
#ERROR define k as an integer between 3 and 8
#endif
#ifndef kString
#ERROR must define kString as an string between "3" and "8" corresponding to value of k above
#endif

typedef unsigned char kperm[3]; // 3 bits per permutation, max 8 permutations = 24 bits
#define Bk (1 <<(k*(k-1)/2))
short int K[Bk]; // does not NEED to be unsigned, so leave it signed.
kperm Permutations[Bk];

void ExtractPerm(char perm[k], int i) // you provide a permutation array, we fill it with permutation i
{
    int j, i32 = 0;
    for(j=0;j<3;j++) i32 |= (Permutations[i][j] << j*8);
    for(j=0;j<k;j++)
	perm[j] = (i32 >> 3*j) & 7;
}

void EncodePerm(kperm *p, char perm[k]) // you provide 3 bytes of storage and a permutation.
{
    int j, i32=0;
    for(j=0;j<k;j++)
	i32 |= (perm[j] << (3*j));
    for(j=0;j<3;j++) (*p)[j] = ((i32 >> j*8) & 255);
}

static int siCmp(const void *A, const void *B)
{
    const int *a = A, *b = B;
    return *a-*b;
}

short int canon2ordinal(int numCanon, int canon_list[numCanon], int canonical)
{
    int *found = bsearch(&canonical, canon_list, numCanon, sizeof(canon_list[0]), siCmp);
    return found-canon_list;
}

int main(int argc, char *argv[])
{
    FILE *fp_ord=fopen("data/canon_list" kString ".txt","r");
    assert(fp_ord);
    int numCanon;
    fscanf(fp_ord, "%d",&numCanon);
    int canon_list[numCanon];
    for(int i=0; i<numCanon; i++) fscanf(fp_ord, "%d", &canon_list[i]);
    fclose(fp_ord);

    FILE *fp=fopen("data/canon_map" kString ".txt","r");
    assert(fp);
    int line;
    for(line=0; line < Bk; line++) {
	int canonical, ordinal;
	char perm[9];
	fscanf(fp, "%d%s",&canonical,perm);
	ordinal=canon2ordinal(numCanon, canon_list, canonical);
	K[line]=ordinal;
	for(int i=0;i<k;i++)perm[i] -= '0';
	EncodePerm(&Permutations[line], perm);
#if 0  // test output?
	for(int i=0;i<k;i++)perm[i]=0;
	ExtractPerm(perm, line);
	printf("K[%d]=%d;", line, K[line]);
	for(int i=0;i<k;i++)printf(" %d", perm[i]);
	printf("\n");
#endif
    }
    fclose(fp);
    fp=fopen("data/canon_map" kString ".bin","wb");
    fwrite((void*)K,sizeof(K[0]),Bk,fp);
    fclose(fp);
    fp=fopen("data/perm_map" kString ".bin","wb");
    fwrite((void*)Permutations,sizeof(Permutations[0]),Bk,fp);
    fclose(fp);
}

