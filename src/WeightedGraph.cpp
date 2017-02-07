#include "WeightedGraph.hpp"
#include "Graph.hpp"
#include <cstdio>
using namespace std;

//public
WeightedGraph::WeightedGraph():Graph(),
    adjMatrix(vector<vector<ushort>>(0))
    {}

void WeightedGraph::saveInGWFormat(string outputFile) {
    uint numNodes = getNumNodes();
    vector<string> nodeNames(numNodes);
    for (uint i = 0; i < numNodes; i++) {
        nodeNames[i] = "node"+intToString(i);
    }
    saveInGWFormat(outputFile, nodeNames, edgeList);
}

void WeightedGraph::saveInGWFormat(string outputFile, const vector<string>& nodeNames,
    const vector<vector<ushort>>& edgeList) {
    ofstream outfile;
    outfile.open(outputFile.c_str());

    writeGWHeader(outfile);
    writeGWNodes(outfile, nodeNames);
    writeGWEdges(outfile);
}

void WeightedGraph::writeGWEdges(ofstream& outfile) {
    uint numEdges = edgeList.size();
    outfile << numEdges << endl;
    for (uint i = 0; i < numEdges; i++)  {
        ushort node1 = edgeList[i][0];
        ushort node2 = edgeList[i][1];
        outfile << node1+1 << ' ' <<  node2+1 << ' ' << adjMatrix[node1][node2]
            << " |{}|" << endl;
    }
}
//void WeightedGraph::edgeList2gw(string fin, string fout) {
//    vector<string> nodes = removeDuplicates(fileToStrings(fin));
//    map<string,uint> nodeName2IndexMap;
//    uint numNodes = nodes.size();
//    for (uint i = 0; i < numNodes; i++) {
//        nodeName2IndexMap[nodes[i]] = i;
//    }
//    vector<vector<string> > edges = fileToStringsByLines(fin);
//    vector<vector<ushort>> edgeList(edges.size(), vector<ushort> (2));
//
//    for (uint i = 0; i < edges.size(); i++) {
//        if (edges[i].size() != 2) {
//            throw runtime_error("File not in edge-list format: "+fin);
//        }
//        string node1 = edges[i][0];
//        string node2 = edges[i][1];
//        uint index1 = nodeName2IndexMap[node1];
//        uint index2 = nodeName2IndexMap[node2];
//        edgeList[i][0] = index1;
//        edgeList[i][1] = index2;
//    }
//    saveInGWFormat(fout, nodes, edgeList);
//}
