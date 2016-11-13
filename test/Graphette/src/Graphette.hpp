#ifndef GRAPHETTE_HPP
#define GRAPHETTE_HPP

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
	Graphette(uint n, uint decimalNumber);
	Graphette(uint n, std::vector<bool>& bitVector);
	Graphette(uint n, HalfMatrix adjMatrix);
	~Graphette();
	uint getDecimalNumber();
	std::vector<bool> getBitVector();
	uint getNumNodes();
	uint getNumEdges();
	uint getDegree(uint node);

	std::vector<Graphette*> static generateAll(uint n);

	std::vector<std::vector<uint>> getOrbits();
	void printAdjMatrix();
private:
	uint numNodes_;
	uint numEdges_, decimalNumber_;
	HalfMatrix adjMatrix_;
	std::vector<uint> degree_;
	uint decodeHalfMatrix();
	void init();
	
	bool suitable(std::vector<uint>& permutation);
	Graphette* permuteNodes(std::vector<uint>& permutation);
	void captureCycles(std::vector<uint>& permutation, std::vector<uint>& orbit);
	void followTrail(std::vector<uint>& permutation, std::vector<uint>& cycle,
						uint seed, uint current, std::vector<bool>& visited);
};
#endif