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

BinaryGraph Utility::LoadBinaryGraphFromLEDAFile(const string &fileName) {
    Utils::checkFileExists(fileName);
    BinaryGraph graph;
    graph.setName(fileName);
  
    stringstream errorMsg;
    ifstream infile(fileName.c_str());
    string token;

    vector<string> LEDAFileFormatExpectation = {"LEDA.GRAPH", "string", "short", "-2"};
    for (int i = 0; i < 4; i++) {
        getline(infile, token);
        if (LEDAFileFormatExpectation[i] != token) {
            errorMsg << "Failed to read graph (" << fileName << ")" 
                     << ". At line " << i << " expected: " 
                     << LEDAFileFormatExpectation[i]
                     << ", got: " << token << ".";
            throw runtime_error(errorMsg.str().c_str());
        }
    }

    int numNodes;
    infile >> numNodes;
    graph.SetNumNodes(numNodes);

    for (int i = 0; i < numNodes; i++) {
        infile >> token;
        graph.SetNodeName(i, token);
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

BinaryGraph Utility::LoadBinaryGraphFromEdgeList(const string &fileName) {
    // Todo: replace using this, and do making graph in one pass
    // Utils::checkFileExists(fileName);
    vector < vector<string> > edges = Utils::fileToStringsByLines(fileName);

    vector <string> nodes;
    unordered_map <string, ushort> nodeNameToIndexMap;

    // Count number of nodes and index them
    for (int i = 0; i < edges.size(); i++) {
        string node1 = edges[i][0];
        string node2 = edges[i][1];

        if (nodeNameToIndexMap.find(node1) == nodeNameToIndexMap.end()) {
            nodeNameToIndexMap[node1] = nodes.size();
            nodes.push_back(node1);
        }

        if (nodeNameToIndexMap.find(node2) == nodeNameToIndexMap.end()) {
            nodeNameToIndexMap[node2] = nodes.size();
            nodes.push_back(node2);
        }
    }

    BinaryGraph graph;
    graph.setName(fileName);
    graph.SetNumNodes(nodes.size());

    for(int i = 0; i < nodes.size(); i++)
        graph.SetNodeName(i, nodes[i]);

    for(int i = 0; i < edges.size(); i++){
        uint node1 = nodeNameToIndexMap[edges[i][0]];
        uint node2 = nodeNameToIndexMap[edges[i][1]];

        graph.AddEdge(node1, node2);
    }

    return graph;
}

PairwiseAlignment Utility::LoadPairwiseAlignmentFromEdgeList(Graph *G1, Graph *G2, const string &filename) {
    unordered_map<string,ushort> mapG1 = G1->getNodeNameToIndexMap();
    unordered_map<string,ushort> mapG2 = G2->getNodeNameToIndexMap();
    vector < vector <string> > mapList = Utils::fileToStringsByLines(filename);
    ushort n1 = mapList.size();
    ushort n2 = G2->GetNumNodes();
    PairwiseAlignment Alig;
    vector<ushort> init(G1->GetNumNodes(),n2);
    Alig.setVector(init);
    for(size_t i = 0; i < n1; ++i) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];
        Alig.getVec()[mapG1[nodeG1]] = mapG2[nodeG2];
    }
    return Alig;
}

