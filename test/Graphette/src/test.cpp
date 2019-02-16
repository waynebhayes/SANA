#include "Graphette.hpp"
#include "HalfMatrix.hpp"
#include "Graph.hpp"
#include "utils/xrand.hpp"
#include "Database.hpp"
#include <bits/stdc++.h>


using namespace std;

int main(int arg, char*argv[]){
#if 0 // Compute distanceMatrix edgeHammingDistance
	int k=atoi(argv[1]);
	distanceMatrix(k, k*(k-1)/2);
#else
    if(arg != 4){cerr << "USAGE: $0 k [-]numSamples edgeListFile; if numSamples is negative then we also output orbits\n"; exit(1);}
    Database d = Database(atoi(argv[1]));
    d.addGraph(argv[3], atoi(argv[2]));
    return 0;
#endif
}
