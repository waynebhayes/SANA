#ifndef GRAPH_HPP
#define GRAPH_HPP
#include "HalfMatrix.hpp"
#include "Graphette.hpp"
#include "utils/xrand.hpp"
#include <bits/stdc++.h>

class Graph{
public:
    Graph();
    Graph(ullint numNodes);
    
    /*
    IMPORTANT: 
        -edgeList will be processed as it is given.
        -Duplicate edges will not be counted.
        -Self-edges are not allowed.
        -No label is allowed. Nodes have to be unsigned integers.
    */
    Graph(std::vector<std::pair<ullint, ullint>>& edgeList);
    
    Graph(HalfMatrix& adjMatrix);
    ~Graph();

    void addEdge(ullint node1, ullint node2);
    void removeEdge(ullint node1, ullint node2);
    bool hasEdge(ullint node1, ullint node2);
    ullint numNodes();
    ullint numEdges();
    ullint degree(ullint node);
    void printAdjMatrix();
    
    /*
    Returns all the nodes within neighborhood of node up to radius.
    Example:
        -For radius=1, it will return the node itself(since distance from node
        is 0) and its direct neighbors. 
        -For radius=2, it will return the neighbors of neighbors, the direct 
        neighbors and the node itself.
    */
    std::vector<ullint> neighbors(ullint node);
    
    /*
    Randomly selects a node and all nodes in its neighborhood within given 
    samplingRadius. Then randomly selects a k-Graphette from these nodes.
    */
    Graphette* sampleGraphette(ullint k, ullint node1, ullint node2);

private:
    ullint numNodes_, numEdges_;
    HalfMatrix adjMatrix_;
    std::vector<std::vector<ullint>> adjList_;
    Graphette* createGraphette(std::vector<ullint>& nodes);
};
#endif