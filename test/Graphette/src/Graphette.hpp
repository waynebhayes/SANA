#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include "HalfMatrix.hpp"

class Graphette{
public:
	Graphette(); //default constructor
	Graphette(ushort n, uint decimalNumber);
	Graphette(ushort n, std::vector<bool>& bitVector);
	Graphette(ushort n, HalfMatrix adjMatrix);
	~Graphette();
	uint getDecimalNumber();
	std::vector<bool> getBitVector();
	ushort getNumNodes();
	uint getNumEdges();
	ushort getDegree(ushort node);

	std::vector<Graphette*> static generateAll(ushort n);

	//Saves orbitID map for canonical graphettes in orbitIds'n'.txt
	void static mapOrbitIds(ushort n, std::vector<uint>& canonnicalGraphettes, std::string filename = "");
	std::vector<std::vector<ushort>> getOrbits();
	void printAdjMatrix();
private:
	ushort numNodes_;
	uint numEdges_, decimalNumber_;
	HalfMatrix adjMatrix_;
	std::vector<ushort> degree_;
	uint decodeHalfMatrix();
	void init();
	
	bool suitable(std::vector<ushort>& permutation);
	Graphette* permuteNodes(std::vector<ushort>& permutation);
	void captureCycles(std::vector<ushort>& permutation, std::vector<ushort>& orbit);
	void followTrail(std::vector<ushort>& permutation, std::vector<ushort>& cycle,
						ushort seed, ushort current, std::vector<bool>& visited);
};