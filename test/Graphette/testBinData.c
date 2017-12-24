#include <sys/file.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if 0
#define k 5
#define kString "5"
#endif

#ifndef k
#ERROR define k as an integer between 3 and 8
#endif
#ifndef kString
#ERROR must define kString as an string between "3" and "8" corresponding to value of k above
#endif

typedef unsigned char kperm[3]; // 3 bits per permutation, max 8 permutations = 24 bits
#define Bk (1 <<(k*(k-1)/2))
kperm Permutations[Bk] __attribute__ ((aligned (32768)));
short int K[Bk] __attribute__ ((aligned (32768)));

void ExtractPerm(char perm[k], int i) // you provide a permutation array, we fill it with permutation i
{
    int j, i32 = 0;
    for(j=0;j<3;j++) i32 |= (Permutations[i][j] << j*8);
    for(j=0;j<k;j++)
	perm[j] = (i32 >> 3*j) & 7;
}

int N=10;
int main(int argc, char *argv[])
{
    FILE *fp_ord=fopen("data/canon_list" kString ".txt","r");
    assert(fp_ord);
    int numCanon;
    fscanf(fp_ord, "%d",&numCanon);
    int canon_list[numCanon];
    for(int i=0; i<numCanon; i++) fscanf(fp_ord, "%d", &canon_list[i]);
    fclose(fp_ord);

    char perm[9];
    int Kfd = open(("data/canon_map" kString ".bin"), 0*O_RDONLY);
    int pfd = open(("data/perm_map" kString ".bin"), 0*O_RDONLY);
    short int *Kf = mmap(K, Bk*sizeof(K[0]), PROT_READ, MAP_PRIVATE|MAP_FIXED, Kfd, 0);
    kperm *Pf = mmap(Permutations, Bk*sizeof(Permutations[0]), PROT_READ, MAP_PRIVATE|MAP_FIXED, pfd, 0);
    if(Kf == MAP_FAILED) perror("mmap Kf");
    if(Pf == MAP_FAILED) perror("mmap Pf");
    if(argc>1)N=atoi(argv[1]);
    for(int n=0; n<N;n++) {
	extern double drand48();
	int g = Bk*drand48(); // find a graphette
	for(int i=0;i<k;i++)perm[i]=0;
	ExtractPerm(perm, g);
	printf("K[%d]=%d [%d];", g, K[g], canon_list[K[g]]);
	for(int i=0;i<k;i++)printf(" %c", perm[i]+'0');
	printf("\n");
    }
}

