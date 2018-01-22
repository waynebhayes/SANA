#include "Utility.hpp"
#include "BinaryGraph.hpp"
#include "Graph.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

Graph Utility::LoadGraphFromLEDAFile(const string &fileName) {
    BinaryGraph graph;

    stringstream errorMsg;
    ifstream infile(fileName.c_str());
    string token;

    vector<string> LEDAFileFormatExpectation = {"LEDA.GRAPH", "string", "short", "-2"};
    for (int i = 0; i < 4; i++) {
        if (LEDAFileFormatExpectation[i++] != token) {
            errorMsg << "Failed to read graph : expected correct LEDA graph format";
            throw runtime_error(errorMsg.str().c_str());
        }
    }
    int numNodes;
    infile >> numNodes;
    graph.SetNumNodes(numNodes);

    for (int i = 0; i < numNodes; i++) {
        infile >> token;
    }

    int numEdges;
    infile >> numEdges;
    for (int i = 0; i < numEdges; i++) {
        unsigned int node1, node2;
        infile >> node1 >> node2 >> token >> token;
        graph.AddEdge(node1, node2);
    }

    return graph;
}

Graph Utility::LoadGraphFromEdgeList(const string &fileName, const string delimiter = "") {
    BinaryGraph graph;
    return graph;
}
