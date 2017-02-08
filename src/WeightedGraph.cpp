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
        outfile << node1+1 << ' ' <<  node2+1 << " 0 |{" << adjMatrix[node1][node2]
            << "}|" << endl;
    }
}

void WeightedGraph::loadGwFile(const string& fileName) {
    //this function could be improved to deal with blank lines and comments
    stringstream errorMsg;

    ifstream infile(fileName.c_str());
    string line;
    //ignore header
    for (int i = 0; i < 4; i++) getline(infile, line);
    //read number of nodes
    int n;
    getline(infile, line);
    istringstream iss(line);
    if (!(iss >> n) or n <= 0) {
        errorMsg << "Failed to read node number: " << line;
        throw runtime_error(errorMsg.str().c_str());
    }
    // Throw away node names
    string node;
    for (int i = 0; i < n; i++) {
        getline(infile, line);
        istringstream iss(line);
        if (!(iss >> node)) {
            errorMsg << "Failed to read node " << i << " of " << n << ": " << line << " (" << node << ")";
            throw runtime_error(errorMsg.str().c_str());
        }
    }
    // Read number of edges
    int m;
    getline(infile, line);
    istringstream iss2(line);
    if (!(iss2 >> m)) {
        errorMsg << "Failed to read edge number: " << line;
        throw runtime_error(errorMsg.str().c_str());
    }

    adjLists = vector<vector<ushort> > (n, vector<ushort>(0));
    adjMatrix = vector<vector<ushort> > (n, vector<ushort>(n, 0));
    edgeList = vector<vector<ushort> > (m, vector<ushort>(2));
    lockedList = vector<bool> (n, false);
    lockedTo = vector<string> (n, "");
    nodeTypes = vector<string> (n, "");

    geneCount = miRNACount = 0;
    //read edges
    for (int i = 0; i < m; i++) {
        getline(infile, line);
        istringstream iss(line);
        ushort node1, node2, reverseEdge, edge;
        char dump;
        if (!(iss >> node1 >> node2 >> reverseEdge >> dump >> dump >> edge)) {
            errorMsg << "Failed to read edge: " << line;
            throw runtime_error(errorMsg.str().c_str());
        }

        node1--; node2--; //-1 because of remapping

        edgeList[i][0] = node1;
        edgeList[i][1] = node2;

        adjMatrix[node1][node2] = adjMatrix[node2][node1] = edge;
        adjLists[node1].push_back(node2);
        adjLists[node2].push_back(node1);
    }
    infile.close();
    initConnectedComponents();
}

