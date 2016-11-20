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
	
	/*
	IMPORTANT: 
		-edgeList will be processed as it is given.
		-Duplicate edges will not be counted.
		-Self-edges are not allowed.
		-No label is allowed. Nodes have to be unsigned integers.
	*/
	Graph(std::vector<std::pair<uint, uint>>& edgeList);
	
	Graph(HalfMatrix& adjMatrix);
	~Graph();

	void addEdge(uint node1, uint node2);
	void removeEdge(uint node1, uint node2);
	bool hasEdge(uint node1, uint node2);
	uint numNodes();
	uint numEdges();
	uint degree(uint node);
	void printAdjMatrix();
	
	/*
	Returns all the nodes within neighborhood of node up to radius.
	Example:
		-For radius=1, it will return the node itself(since distance from node
		is 0) and its direct neighbors. 
		-For radius=2, it will return the neighbors of neighbors, the direct 
		neighbors and the node itself.
	*/
	std::vector<uint> neighbors(uint node, uint radius);
	
	/*
	Randomly selects a node and all nodes in its neighborhood within given 
	samplingRadius. Then randomly selects a k-Graphette from these nodes.
	*/
	Graphette* sampleGraphette(uint k, uint samplingRadius);

private:
	uint numNodes_, numEdges_;
	HalfMatrix adjMatrix_;
	std::vector<uint> degree_;
	/*
	For a given seed, it explores its neighborhood to distance up to radius.
	*/
	std::set<uint> explore(uint seed, uint radius, std::vector<bool>& visited);
	Graphette* createGraphette(std::vector<uint>& nodes);
};

#endif