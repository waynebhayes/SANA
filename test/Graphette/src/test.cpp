#include "Graphette.hpp"
#include "HalfMatrix.hpp"
#include "Graph.hpp"
#include "utils/xrand.hpp"
#include "Database.hpp"
#include <bits/stdc++.h>


using namespace std;

int main(int arg, char*argv[]){
	if(arg != 5){cerr << "USAGE: $0 k radius numSamples edgeListFile\n"; exit(1);}
	Database d = Database(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	d.addGraph(argv[4]);
	return 0;
}
