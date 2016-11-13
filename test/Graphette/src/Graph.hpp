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
#include "HalfMatrix.hpp"

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
	uint getNumNodes();
	uint getNumEdges();
	uint getDegree(uint node);
	void printAdjMatrix();
	Graph* getSubGraph(std::vector<uint>& nodes);
private:
	uint numNodes_, numEdges_;
	HalfMatrix adjMatrix_;
	std::vector<uint> degree_;
};

#endif