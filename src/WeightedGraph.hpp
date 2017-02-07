#ifndef WEIGHTED_GRAPH_H
#define WEIGHTED_GRAPH_H
#include "Graph.hpp"

class WeightedGraph : public Graph {

public:
    WeightedGraph();
    WeightedGraph(const Graph& G);
    WeightedGraph(const WeightedGraph &G);

    void saveInGWFormat(string outputFile);
    void saveInGWFormat(string outputFile, const vector<string>& nodeNames,
            const vector<vector<ushort>>& edgeList);
    void writeGWEdges(ofstream& outfile);
private:
    vector<vector<ushort>> adjMatrix;
};
#endif
