#ifndef GRAPH_H
#define GRAPH_H
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <map>
using namespace std;

class Graph {

public:
    //creates the null graph
    Graph();
    Graph(const Graph& G);


    Graph(uint n, const vector<vector<ushort> > edgeList);

    string getName() const;
    
    void reset();
    uint getNumNodes() const;
    uint getNumEdges() const;
    const vector<vector<ushort> >& getConnectedComponents() const;
    uint getNumConnectedComponents() const;
    void getAdjMatrix(vector<vector<bool> >& adjMatrixCopy) const;
    void getAdjLists(vector<vector<ushort> >& adjListsCopy) const;
    void getEdgeList(vector<vector<ushort> > & edgeListCopy) const;
    vector<string> getNodeNames() const;

    //loads graph from file in GraphWin (.gw) format:
    //http://www.algorithmic-solutions.info/leda_guide/graphs/leda_native_graph_fileformat.html
    //note: it does not parse correctly files with comments or blank lines.
    void loadGwFile(const string& fileName);

    //nodes are relabeled so that the new i-th node is the node nodes[i]-th in this
    Graph nodeInducedSubgraph(const vector<ushort>& nodes) const;

    uint numNodeInducedSubgraphEdges(const vector<ushort>& subgraphNodes) const;

    vector<ushort> numEdgesAround(ushort node, ushort maxDist) const;
    vector<ushort> numNodesAround(ushort node, ushort maxDist) const;

    void printStats(int numConnectedComponentsToPrint, ostream& stream) const;

    void writeGraphEdgeListFormat(const string& fileName);
    void writeGraphEdgeListFormatNETAL(const string& fileName);

    void addRandomEdges(double addedEdgesProportion);
    void removeRandomEdges(double removedEdgesProportion);
    void rewireRandomEdges(double rewiredEdgesProportion);

    ushort randomNode();

    vector<vector<uint> > loadGraphletDegreeVectors();

    map<string,ushort> getNodeNameToIndexMap() const;
    map<ushort,string> getIndexToNodeNameMap() const;

    void getDistanceMatrix(vector<vector<short> >& dist) const;

    vector<uint> degreeDistribution() const;

    double getAverageDistance() const;

    void saveInGWFormat(string outputFile);
    void saveInGWFormatWithNames(string outputFile);
    void saveInShuffledOrder(string outputFile);
    void saveGraphletsAsSigs(string outputFile);

    Graph randomNodeInducedSubgraph(uint numNodes);

    bool isWellDefined();

    bool sameNodeNames(const Graph& other) const;

    // For locking
    void setLockedList(vector<string>& nodes, vector<string>& pairs);
    vector<bool>& getLockedList();
    bool isLocked(uint index);
    string getLockedTo(uint index);
    int getLockedCount();

    //My functions and constructor
    Graph(int n);
    void setAdjMatrix(vector<bool>& v);
    void print_adjMatrix(bool upper = 0);
    uint getDegree(uint node) const;
    void set_decimal_representation(int n);
    int get_decimal_representation() const;
    void construct_decimal_representation();
    void print_adjLists();

private:

    string name;

    vector<vector<ushort> > edgeList; //edges in no particular order
    vector<vector<bool> > adjMatrix;
    vector<vector<ushort> > adjLists; //neighbors in no particular order
    int decimal_representation;

    //list of the nodes of each connected component, sorted from larger to smaller
    vector<vector<ushort> > connectedComponents;
    vector<bool> lockedList;  // shows which nodes are locked
    vector<string> lockedTo;  // name of node we lock to in other graph
    int lockedCount = 0;


    void initConnectedComponents();

    void addEdge(ushort node1, ushort node2);
    void removeEdge(ushort node1, ushort node2);
    void addRandomEdge();
    void removeRandomEdge();

    string autogenFilesFolder();

    vector<vector<uint> > computeGraphletDegreeVectors();

    //places in dist a matrix with the distance between every pair of nodes (a -1 indicates infinity)
    void computeDistanceMatrix(vector<vector<short> >& dist) const;

};

#endif
