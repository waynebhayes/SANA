#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <map>
#include <unordered_map>
#include <iostream>
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
    unsigned int numNodes;
    unsigned int numEdges;
    vector<vector<unsigned int> > adjLists;
    vector<string> nodeNames;
    string name;
public:
    Graph():numNodes(0),numEdges(0){};
    ~Graph(){};

    unordered_map<string,ushort> getNodeNameToIndexMap() const;
    unordered_map<ushort,string> getIndexToNodeNameMap() const;
    const vector < vector<unsigned int> >& getAdjList() const;
    void getCopyAdjList(vector < vector<ushort> > &adjListCopy) const;

    void genAdjMatrix(vector < vector<bool> > &adjMatrixCopy) const;

    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight = 1) ;
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2);

    virtual void AddRandomEdge();
    virtual void RemoveRandomEdge();
    virtual int RandomNode();

    unsigned int GetNumNodes() const;
    unsigned int GetNumEdges() const;
    unsigned int NumNodeInducedSubgraphEdges(const vector <ushort>& subgraphNodes) const;
    string GetName() const;

    virtual void SetNumNodes(const unsigned int &);
    void setName(const string& name);

    string GetNodeName(const unsigned int &nodeIndex) const;

    void SetNodeName(const unsigned int &nodeIndex, const string &name);
    virtual void ClearGraph();
};


#endif
