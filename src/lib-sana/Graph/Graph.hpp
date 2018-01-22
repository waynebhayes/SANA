#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>
using namespace std;

class Graph {
public:
    Graph(){};
    ~Graph(){};

    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight = 1);
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2);

    unsigned int GetNumNodes() const;
    unsigned int GetNumEdges() const;

    virtual void SetNumNodes(const unsigned long long int &);

    string getName() const;
    
private:
    int numNodes;
    int numEdges;
    vector<vector<unsigned int> > adjLists;

    string name;

};

#endif
