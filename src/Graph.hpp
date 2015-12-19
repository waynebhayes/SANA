#ifndef GRAPH_H
#define GRAPH_H
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include "utils.hpp"
using namespace std;

class Graph {

public:

    static Graph loadGraph(string name);


    static void saveInGWFormat(string outputFile, const vector<string>& nodeNames,
        const vector<vector<bool> >& adjMatrix);
    static void edgeList2gw(string fin, string fout);
    
    static void GeoGeneDuplicationModel(uint numNodes, uint numEdges, string outputFile);


    //creates the null graph
    Graph();
    Graph(const Graph& G);


    Graph(uint n, const vector<vector<ushort> > edgeList);

    string getName() const;
    
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
    Graph randomNodeInducedSubgraph(uint numNodes);

    bool isWellDefined();

private:

    string name;

    vector<vector<ushort> > edgeList; //edges in no particular order
    vector<vector<bool> > adjMatrix;
    vector<vector<ushort> > adjLists; //neighbors in no particular order

    //list of the nodes of each connected component, sorted from larger to smaller
    vector<vector<ushort> > connectedComponents;

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
