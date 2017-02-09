#ifndef WEIGHTED_GRAPH_H
#define WEIGHTED_GRAPH_H
#include "Graph.hpp"

class WeightedGraph : public Graph {

public:
    WeightedGraph();
    WeightedGraph(const Graph& G);
    WeightedGraph(const WeightedGraph &G);

    void saveInGWFormat(string outputFile);
    static void saveInGWFormat(string outputFile, const vector<string>& nodeNames,
            const vector<vector<ushort>>& edgeList, const vector<vector<ushort>>& adjMatrix);

    void loadGwFile(const string& fileName);

    void getAdjMatrix(vector<vector<ushort>>& adjMatrixCopy) const;

    WeightedGraph nodeInducedSubgraph(const vector<ushort>& nodes) const;

    void printStats(int numConnectedComponentsToPrint, ostream& stream) const;

    // Commented code is to be implemented later
    // static void saveInGWFormatShuffled(string outputFile, const vector<string>& nodeNames,
    //    const vector<vector<ushort>>& edgeList);
    // Not sure?
    // static void edgeList2gw(string fin, string fout);
    // static void GeoGeneDuplicationModel(uint numNodes, uint numEdges, string outputFile);
    // void multGwFile(const string& fileName, uint path);
    // void writeGraphEdgeListFormat(const string& fileName);
    // void writeGraphEdgeListFormatNETAL(const string& fileName);
    // void writeGraphEdgeListFormatPISWAP(const string& fileName);
    // void writeGraphEdgeListFormatPINALOG(const string& fileName);
    //
    void addRandomEdges(double addedEdgesProportion);
    void removeRandomEdges(double removedEdgesProportion);
    void rewireRandomEdges(double rewiredEdgesProportion);
protected:
    void addEdge(ushort node1, ushort node2);
    void removeEdge(ushort node1, ushort node2);
    void addRandomEdge();
    void removeRandomEdge();
    
private:
    static void writeGWEdges(ofstream& outfile, const vector<vector<ushort>>& edgeList, 
                                const vector<vector<ushort>>& adjMatrix);
    vector<vector<ushort>> adjMatrix;
};
#endif
