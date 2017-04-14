#include "Graphette.hpp"
#include "HalfMatrix.hpp"
#include "Graph.hpp"
#include "utils/xrand.hpp"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <utility>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include "Database.hpp"

using namespace std;

int main(int arg, char*argv[]){
	if(arg != 5){cerr << "USAGE: $0 k radius numSamples edgeListFile\n"; exit(1);}
	Database d = Database(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	d.addGraph(argv[4]);
	return 0;
}
