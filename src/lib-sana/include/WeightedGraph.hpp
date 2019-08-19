#ifndef WEIGHTEDGRAPH_HPP
#define WEIGHTEDGRAPH_HPP
#include "Graph.hpp"
#include <vector>

using namespace std;

class WeightedGraph : public Graph {
public:
    WeightedGraph(){};
    ~WeightedGraph(){};
    
    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight = 1);
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2);

    virtual void SetNumNodes(const unsigned int &numNodes);

    virtual void ClearGraph();
private:
    vector<vector<unsigned int> > adjacencyMatrix;
};

#endif
