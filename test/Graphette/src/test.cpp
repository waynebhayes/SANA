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
#include <ctime>
#include "utils/xrand.hpp"

using namespace std;

int main(int arg, char*argv[]){
	uint m, n, k = atoi(argv[1]), radius = atoi(argv[2]), limit= atoi(argv[3]);
	vector<pair<uint, uint>> edgelist;
	ifstream fin("test/edgelist.txt"), forbit("data/orbit_map"+to_string(k)+".txt");
	string temp;
	getline(forbit, temp);
	while(fin >> m >> n){
		edgelist.push_back(make_pair(m, n));
	}
	forbit >> m;
	Graph g(edgelist);
	vector<vector<bool>> sigMatrix(g.numNodes(), vector<bool>(m, false));//each row for each node
	
	for(uint i=0; i<limit; i++){
		Graphette* gh = g.sampleGraphette(k, radius);

		delete gh;
	}
	cout << endl;
	return 0;
}