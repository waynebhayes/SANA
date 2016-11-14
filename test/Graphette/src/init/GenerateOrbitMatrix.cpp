#include "../Graphette.hpp"
#include <cstdlib>
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void generateOrbitMatrix(uint n){
	string inname = "data/canon_list"+to_string(n)+".txt";
	string outname = "data/orbit_map"+to_string(n)+".txt";
	ifstream fin(inname);
	ofstream fout(outname);
	uint num, cgraph, orbitId = 0;
	vector<uint> canonical;

	fin >> num;
	fout << "#ORBIT ID FOR EACH NODE OF EACH " << n << "-CANONICAL GRAPHETTE IN ASCENDING ORDER\n";
	while(fin >> cgraph){
		Graphette g = Graphette(n, cgraph);
		vector<uint> idList(n);		
		vector<vector<uint>> orbits = g.orbits();
		for(auto orbit : orbits){
			for(auto node : orbit){
				idList[node] = orbitId;
			}
			orbitId++;
		}
		for(uint i=0; i < idList.size(); i++){
				fout << idList[i] << " ";
			}
		fout << endl;
	}
}