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
}