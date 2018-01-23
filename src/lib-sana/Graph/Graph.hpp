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

    uint getGeneCount() const;
    void setGeneCount(uint geneCount);

    uint getMiRNACount() const;
    void setMiRNACount(uint miRNACount);

    int getUnlockedGeneCount() const;
    void setUnlockedGeneCount(int unlockedGeneCount);

    int getUnlockedmiRNACount() const;
    void setUnlockedmiRNACount(int unlockedmiRNACount);

    const vector<string> &getNodeTypes() const;
    void addNodeType(const string &nodeType);

private:
    uint geneCount = 0;
    uint miRNACount = 0;
    int unlockedGeneCount = -1;
    int unlockedmiRNACount = -1;
    vector <string> nodeTypes;

private:

    unsigned int numNodes;
    unsigned int numEdges;
    vector<vector<unsigned int> > adjLists;

    string name;

};

#endif
