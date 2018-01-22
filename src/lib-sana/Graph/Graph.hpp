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
    virtual void AddRandomEdge();
    virtual void RemoveRandomEdge();
    virtual int RandomNode();

    unsigned int GetNumNodes() const;
    unsigned int GetNumEdges() const;
    string getName() const;

    virtual void SetNumNodes(const unsigned int &);
    void setName(string name);


private:
    int numNodes;
    int numEdges;
    vector<vector<unsigned int> > adjLists;

    string name;

};

#endif
