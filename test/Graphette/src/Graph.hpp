#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <set>
#include "HalfMatrix.hpp"
#include "Graphette.hpp"
#include "utils/xrand.hpp"

class Graph{
public:
	Graph();
	Graph(uint numNodes);
	//IMPORTANT: edgeList will be processed as it is given.
	//Duplicate edges will not be removed.
	//Self-edges are not allowed
	//Nodes have to be unsigned integers
	Graph(std::vector<std::pair<uint, uint>>& edgeList);
	Graph(HalfMatrix& adjMatrix);
	~Graph();

	void addEdge(uint node1, uint node2);
	void removeEdge(uint node1, uint node2);
	bool getEdge(uint node1, uint node2);
	uint getNumNodes();
	uint getNumEdges();
	uint getDegree(uint node);
	void printAdjMatrix();
	Graphette* getSampleGraphette(uint numNodes, uint radius = 3);
	std::vector<uint> getNeighbors(uint node, uint radius);
private:
	uint numNodes_, numEdges_;
	HalfMatrix adjMatrix_;
	std::vector<uint> degree_;
	std::set<uint> explore(uint seed, uint radius);
};

#endif