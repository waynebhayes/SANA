#include "Graphette.hpp"
#include "HalfMatrix.hpp"
#include "Graph.hpp"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <utility>
#include "utils/xrand.hpp"

using namespace std;

int main(int arg, char*argv[]){
	uint m, n, seed = atoi(argv[1]), radius = atoi(argv[2]), len= atoi(argv[3]);
	vector<pair<uint, uint>> edgelist;
	ifstream fin("test/edgelist.txt");
	while(fin >> m >> n){
		edgelist.push_back(make_pair(m, n));
	}
	Graph g(edgelist);
	auto nbors = g.neighbors(seed, radius);
	for(auto nbor: nbors)
		cout << nbor << " ";
	cout << endl;
	xshuffle(nbors, len);
	//nbors.resize(len);
	for(auto nbor: nbors)
		cout << nbor << " ";
	cout << endl;
	return 0;
}