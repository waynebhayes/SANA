#ifndef BINARYGRAPH_HPP
#define BINARYGRAPH_HPP
#include "Graph.hpp"
#include <vector>

using namespace std;

class BinaryGraph : protected Graph {
public:
    BinaryGraph(){};
    ~BinaryGraph(){};

    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight = 1);
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2);

private:
    vector<vector<bool> > adjacencyMatrix;
};

#endif