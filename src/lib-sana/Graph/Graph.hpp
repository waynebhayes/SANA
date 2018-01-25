#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>

using namespace std;

class GraphInvalidIndexError : public exception {
public:
    GraphInvalidIndexError(const string &message): reason(message) {};
    virtual const char* what() const throw() {
        return reason.c_str();
    }
private:
    string reason;
};

class Graph {
public:
    Graph(){};
    ~Graph(){};

    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight = 1) throw(GraphInvalidIndexError) ;
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2) throw(GraphInvalidIndexError);

    unsigned int GetNumNodes() const;
    unsigned int GetNumEdges() const;

    virtual void SetNumNodes(const unsigned int &);

    string GetGraphName() const;
    string GetNodeName(const unsigned int &nodeIndex) const throw(GraphInvalidIndexError);

    void SetNodeName(const unsigned int &nodeIndex, const string &name) throw(GraphInvalidIndexError);

    virtual void ClearGraph();
private:
    int numNodes;
    int numEdges;
    vector<vector<unsigned int> > adjLists;
    vector<string> nodesName;
    string graphName;

};


#endif
