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
public:
    Graph(){};
    ~Graph(){};

    unordered_map<string,ushort> getNodeNameToIndexMap() const;
    unordered_map<ushort,string> getIndexToNodeNameMap() const;

    virtual void AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight) throw(GraphInvalidIndexError);
    virtual void RemoveEdge(const unsigned int &node1, const unsigned int &node2) throw(GraphInvalidIndexError);

    virtual void AddRandomEdge();
    virtual void RemoveRandomEdge();
    virtual int RandomNode();

    unsigned int GetNumNodes() const;
    unsigned int GetNumEdges() const;
    string GetName() const;

    virtual void SetNumNodes(const unsigned int &);
    void setName(string name);

    string GetNodeName(const unsigned int &nodeIndex) const throw(GraphInvalidIndexError);

    void SetNodeName(const unsigned int &nodeIndex, const string &name) throw(GraphInvalidIndexError);
    virtual void ClearGraph();

private:
    unsigned int numNodes;
    unsigned int numEdges;
    vector<vector<unsigned int> > adjLists;
    vector<string> nodeNames;
    string name;

};


#endif
