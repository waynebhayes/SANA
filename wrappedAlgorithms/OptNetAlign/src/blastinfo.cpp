#include <unordered_map>
#include <tuple>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "boost/multi_array.hpp"
#include "Network.h"
#include "blastinfo.h"

BLASTDict* loadBLASTInfo(Network* net1, Network* net2, string filename){

	ifstream infile(filename);
	if(!infile.good()){
		throw LineReadException("Failed to load BLAST info! Check filename.");
	}

	int net1Size = net1->nodeToNodeName.size();
	int net2Size = net2->nodeToNodeName.size();
	BLASTDict* toReturn = new BLASTDict(boost::extents[net1Size][net2Size]);
	for(int i = 0; i < net1Size; i++){
		for(int j = 0; j < net2Size; j++){
			(*toReturn)[i][j] = 0.0;
		}
	}
	string line;
	unsigned int count = 0;

	while(getline(infile,line)){
		istringstream iss(line);

		double blastScore;
		string a, b;
		node u, v;

		if (!(iss >> a >> b >> blastScore)){
			throw LineReadException(string("Parse error in network: ") + filename +
				                   string("on line: ") + line + "\n");
		}
		u = net1->nodeNameToNode[a];
		v = net2->nodeNameToNode[b];
		(*toReturn)[u][v] = blastScore;
		count++;
	}
	return toReturn;
}