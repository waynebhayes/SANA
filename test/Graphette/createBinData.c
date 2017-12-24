#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

int main(int argc, char *argv[])
{
    FILE *fp=fopen("data/canon_map" kString ".txt","r");
    assert(fp);
    int line;
    for(line=0; line < Bk; line++) {
	int map;
	char perm[9];
	fscanf(fp, "%d%s",&map,perm);
	K[line]=map;
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

