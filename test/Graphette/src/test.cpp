#include "Graphette.hpp"
//#include "HalfMatrix.hpp"
#include <cstdlib>
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void printAdjMatrix(ushort numNodes_, HalfMatrix& adjMatrix_){
	for (ushort i = 0; i < numNodes_; i++){
		for (ushort j = 0; j < numNodes_; j++){
			if (i < j)
				cout << adjMatrix_(i, j) << " ";
			else
				cout << "  ";
		}
		cout << "\n";
	}
}

int main(int arg, char*argv[]){
	uint n = atoi(argv[1]);
	string filename = "data/list"+to_string(n)+".txt";
	ifstream fin(filename);
	uint num;
	fin >> num;
	vector<uint> canon;
	while(fin >> num){
		canon.push_back(num);
	}
	Graphette::mapOrbitIds(n, canon);
	//Graphette* g = new Graphette(5, 10);
	//vector<Graphette> g;
	//Graphette::getAll(4, g);
	//for(auto h: g){
	//	h.printAdjMatrix(1);
	//}
	//std::vector<bool> v = g->getBitVector();
	//for(bool i : v) 
	//	cout << i;
	
	/*Graphette g = Graphette(atoi(argv[1]), atoi(argv[2]));
	std::vector<std::vector<ushort>> orbits = g.getOrbits();
	cout << orbits.size() << endl;
	for(auto orbit : orbits){
		cout << "( ";
		for(auto node: orbit)
			cout << node << " ";
		cout << ")  ";
	}
	cout << endl;*/
	//delete g;
	//g->printAdjMatrix();

	/*HalfMatrix a(atoi(argv[1]), atoi(argv[2]));
	HalfMatrix b = a;
	cout << "Printing a=\n";
	printAdjMatrix(atoi(argv[1]), a);
	cout << "\nPrinting b=\n";
	printAdjMatrix(atoi(argv[1]), b);*/
	//for(uint i = 0; i < g->getNumNodes(); i++)
	//	cout << g->getDegree(i);

	//vector<bool> bits;
	//bits = {0, 0, 0 , 1, 1, 1};
	//Graphette g = Graphette(atoi(argv[1]), atoi(argv[2]));
	//cout << g.getDecimalNumber();
}