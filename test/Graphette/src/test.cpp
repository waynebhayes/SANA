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
	/*uint m, n, i, j, numOrbitId, k = atoi(argv[1]), radius = atoi(argv[2]), limit= atoi(argv[3]);
	string str;
	ifstream fedgelist("test/edgelist.txt"), fcanon_map("data/canon_map"+to_string(k)+".txt");
	ifstream forbit_map("data/orbit_map"+to_string(k)+".txt"), fcanon_list("data/canon_list"+to_string(k)+".txt");
	
	vector<pair<uint, uint>> edgelist;
	vector<uint> canonDec; //canonDec[i] = decimal repr. of the canonical isomorph of graphette i;
	vector<uint> canonList; //canonList[i] = ith canonical graphette in decimal
	vector<string> canonPerm; //canonPerm[i] = Permutation of the nodes from  graphette i to canonical isomorph;
	vector<vector<uint>> orbitId; //orbitId[i][j] = orbit id of jth node from ith canon. graphette in ascending order
	
	//reading edgelist
	while(fedgelist >> m >> n){
		edgelist.push_back(make_pair(m, n));
	}
	fedgelist.close();
	
	//reading canon_map
	while(fcanon_map >> i >> str){
		canonDec.push_back(i);
		canonPerm.push_back(str);
	}
	fcanon_map.close();

	//reading canon_list
	fcanon_list >> m; //reading the number of canonical graphettes
	while(fcanon_list >> i){
		canonList.push_back(i);
	}
	fcanon_list.close();

	//reading orbit_map
	forbit_map >> numOrbitId; //reading the number of orbit ids
	for(i = 0; i < canonList.size(); i++){
		vector<uint> inputvec;
		for(j = 0; j < k; j++){
			forbit_map >> m;
			inputvec.push_back(m);
		}
		orbitId.push_back(inputvec);
	}
	forbit_map.close();

	Graph graph(edgelist);
	vector<vector<bool>> sigMatrix(graph.numNodes(), vector<bool>(numOrbitId, false));//each row for each node

	for(i = 0; i<limit; i++){
		Graphette* g = graph.sampleGraphette(k, radius);
		uint gDec = g->decimalNumber();
		uint cgIndex = lower_bound(canonList.begin(), canonList.end(), canonDec[gDec])-canonList.begin(); //index of canonical isomorph of g
		if(canonDec[cgIndex] != canonDec[gDec]){
			throw domain_error("init::init(): canonical graphette not found in canonDec");
		}
		for(j = 0; j < k; j++){
			uint oj = canonPerm[g->decimalNumber()].at(j)-'0'; //cg-er j number place-er noder original index
			uint id = orbitId[index][j];
			sigMatrix[g->label(pj)][id]= true;
		}
		delete g;
	}
	for(auto a: sigMatrix){
		for(auto b: a) cout << b;
		cout << endl;
	}
	return 0;
	*/
	Database d = Database(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	d.addGraph("test/edgelist.txt");
	//ofstream fout("test/1/edge");
	//fout << "tets\n";
	return 0;
}