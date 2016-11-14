#ifndef GRAPHETTE_HPP
#define GRAPHETTE_HPP

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include "HalfMatrix.hpp"

class Graphette{
public:
	Graphette(); //default constructor
	Graphette(uint n, uint decimalNumber);
	Graphette(uint n, std::vector<bool>& bitVector);
	Graphette(uint n, HalfMatrix adjMatrix);
	~Graphette();
	uint decimalNumber();
	std::vector<bool> bitVector();
	uint numNodes();
	uint numEdges();
	uint degree(uint node);
	uint label(uint node);
	std::vector <uint> labels();
	void setLabel(uint node, uint label);
	void setLabels(std::vector<uint>& label);

	std::vector<Graphette*> static generateAll(uint n);
	std::vector<std::vector<uint>> orbits();

private:
	uint numNodes_;
	uint numEdges_, decimalNumber_;
	HalfMatrix adjMatrix_;
	std::vector<uint> degree_, label_;
	uint decodeHalfMatrix();
	void init();
	
	bool suitable(std::vector<uint>& permutation);
	Graphette* permuteNodes(std::vector<uint>& permutation);
	void captureCycles(std::vector<uint>& permutation, std::vector<uint>& orbit);
	void followTrail(std::vector<uint>& permutation, std::vector<uint>& cycle,
						uint seed, uint current, std::vector<bool>& visited);
};
#endif