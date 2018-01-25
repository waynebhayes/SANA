#include "Utility.hpp"
#include "BinaryGraph.hpp"
#include "Utils.hpp"
#include "Graph.hpp"
#include <string>
#include <fstream>
#include <iostream> //Only for debugging
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

Graph Utility::LoadGraphFromLEDAFile(const string &fileName) {
    BinaryGraph graph;
    graph.setName(fileName);
  
    stringstream errorMsg;
    ifstream infile(fileName.c_str());
    string token;

    vector<string> LEDAFileFormatExpectation = {"LEDA.GRAPH", "string", "short", "-2"};
    for (int i = 0; i < 4; i++) {
        getline(infile, token);
        if (LEDAFileFormatExpectation[i] != token) {
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
        graph.AddEdge(node1-1, node2-1);
    }

    return graph;
}

Graph Utility::LoadGraphFromEdgeList(const string &fileName) {
    BinaryGraph graph;
    graph.setName(fileName);

    vector <string> nodes;
    unordered_map <string, ushort> nodeNameIndexMap;
    vector < vector<string> > edges = Utils::fileToStringsByLines(fileName);

    for (int i = 0; i < edges.size(); i++) {
        string node1 = edges[i][0];
        string node2 = edges[i][1];

        if (nodeNameIndexMap.find(node1) == nodeNameIndexMap.end()) {
            nodeNameIndexMap[node1] = nodes.size();
            nodes.push_back(node1);
            graph.addNodeType("gene");
            graph.setGeneCount(graph.getGeneCount()+1);
        }

        if (nodeNameIndexMap.find(node2) == nodeNameIndexMap.end()) {
            nodeNameIndexMap[node2] = nodes.size();
            nodes.push_back(node2);
            graph.addNodeType("miRNA");
            graph.setMiRNACount(graph.getMiRNACount()+1);
        }
    }

    graph.SetNumNodes(nodes.size());
    return graph;
}
