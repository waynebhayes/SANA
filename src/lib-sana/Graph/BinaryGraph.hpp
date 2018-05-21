#ifndef BINARYGRAPH_HPP
#define BINARYGRAPH_HPP
#include "Graph.hpp"
#include <vector>

using namespace std;

class BinaryGraph : public Graph {
public:
    BinaryGraph(){};
    ~BinaryGraph(){};

    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight) throw(GraphInvalidIndexError);
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2) throw(GraphInvalidIndexError);

    virtual void SetNumNodes(const unsigned int &numNodes);
    
    virtual void ClearGraph();

private:
    vector<vector<bool> > adjacencyMatrix;
};

#endif
