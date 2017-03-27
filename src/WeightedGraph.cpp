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
    saveInGWFormat(outputFile, nodeNames, edgeList, adjMatrix);
}

void WeightedGraph::saveInGWFormat(string outputFile, const vector<string>& nodeNames,
    const vector<vector<ushort>>& edgeList, const vector<vector<ushort>>& adjMatrix) {
    ofstream outfile;
    outfile.open(outputFile.c_str());

    writeGWHeader(outfile);
    writeGWNodes(outfile, nodeNames);
    writeGWEdges(outfile, edgeList, adjMatrix);
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
        if (!(iss >> node1 >> node2 >> dump >> dump >> dump >> edge)) {
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

void WeightedGraph::writeGWEdges(ofstream& outfile, const vector<vector<ushort>>& edgeList, 
                                const vector<vector<ushort>>& adjMatrix) {
    uint numEdges = edgeList.size();
    outfile << numEdges << endl;
    for (uint i = 0; i < numEdges; i++)  {
        ushort node1 = edgeList[i][0];
        ushort node2 = edgeList[i][1];
        outfile << node1+1 << ' ' <<  node2+1 << " 0 |{" << adjMatrix[node1][node2]
            << "}|" << endl;
    }
}

void WeightedGraph::getAdjMatrix(vector<vector<ushort>>& adjMatrixCopy) const {
    uint n = getNumNodes();
    adjMatrixCopy = vector<vector<ushort>> (n, vector<ushort> (n));
    for (uint i = 0; i < n; i++) {
        for (uint j = 0; j < n; j++) adjMatrixCopy[i][j] = adjMatrix[i][j];
    }
}

WeightedGraph WeightedGraph::nodeInducedSubgraph(const vector<ushort>& nodes) const {
    uint n = nodes.size();
    vector<ushort> rev = reverseMapping(nodes, getNumNodes());
    unordered_set<ushort> nodeSet(nodes.begin(), nodes.end());
    WeightedGraph G;
    G.adjLists = vector<vector<ushort> > (n, vector<ushort> (0));
    G.adjMatrix = vector<vector<ushort> > (n, vector<ushort> (n, 0));
    //only add edges between induced nodes
    for (const auto& edge: edgeList) {
        ushort node1 = edge[0], node2 = edge[1];
        if (nodeSet.count(node1) and nodeSet.count(node2)) {
            ushort newNode1 = rev[node1];
            ushort newNode2 = rev[node2];
            G.adjLists[newNode1].push_back(newNode2);
            G.adjLists[newNode2].push_back(newNode1);
            vector<ushort> newEdge(2);
            newEdge[0] = newNode1;
            newEdge[1] = newNode2;
            G.edgeList.push_back(newEdge);
            G.adjMatrix[newNode1][newNode2] = G.adjMatrix[newNode2][newNode1] = adjMatrix[node1][node2];
        }
    }
    G.initConnectedComponents();
    return G;
}

void WeightedGraph::printStats(int numConnectedComponentsToPrint, ostream& stream) const {
    stream << "n    = " << getNumNodes() << endl;
    stream << "m    = " << getNumEdges() << endl;
    stream << "#connectedComponents = " << getNumConnectedComponents() << endl;
    stream << "Largest connectedComponents (nodes, edges) = ";
    for (int i = 0; i < min(numConnectedComponentsToPrint, getNumConnectedComponents()); i++) {
        const vector<ushort>& nodes = getConnectedComponents()[i];
        WeightedGraph H = nodeInducedSubgraph(nodes);
        stream << "(" << H.getNumNodes() << ", " << H.getNumEdges() << ") ";
    }
    stream << endl;
}

void WeightedGraph::addEdge(ushort node1, ushort node2) {
    adjMatrix[node1][node2] = adjMatrix[node2][node1] = 1;
    vector<ushort> edge(2);
    edge[0] = node1;
    edge[1] = node2;
    edgeList.push_back(edge);
    adjLists[node1].push_back(node2);
    adjLists[node2].push_back(node1);
}

void WeightedGraph::addRandomEdge() {
    ushort node1 = 0, node2 = 0;
    while (node1 == node2 or adjMatrix[node1][node2]) {
        node1 = randomNode();
        node2 = randomNode();
    }
    addEdge(node1, node2);
}

void WeightedGraph::removeRandomEdge() {
    ushort node1 = 0, node2 = 0;
    while (node1 == node2 or not adjMatrix[node1][node2]) {
        node1 = randomNode();
        node2 = randomNode();
    }
    removeEdge(node1, node2);
}

void WeightedGraph::removeEdge(ushort node1, ushort node2) {
    adjMatrix[node1][node2] = adjMatrix[node2][node1] = 0;
    uint m = getNumEdges();
    //update edge list
    for (uint i = 0; i < m; i++) {
        if ((edgeList[i][0] == node1 and edgeList[i][1] == node2) or
            (edgeList[i][0] == node2 and edgeList[i][1] == node1)) {
            vector<ushort> lastEdge = edgeList[m-1];
            edgeList[i] = lastEdge;
            edgeList.pop_back();
            break;
        }
    }
    //update adjacency lists
    for (uint i = 0; i < adjLists[node1].size(); i++) {
        if (adjLists[node1][i] == node2) {
            ushort lastNeighbor = adjLists[node1][adjLists[node1].size()-1];
            adjLists[node1][i] = lastNeighbor;
            adjLists[node1].pop_back();
            break;
        }
    }
    for (uint i = 0; i < adjLists[node2].size(); i++) {
        if (adjLists[node2][i] == node1) {
            ushort lastNeighbor = adjLists[node2][adjLists[node2].size()-1];
            adjLists[node2][i] = lastNeighbor;
            adjLists[node2].pop_back();
            break;
        }
    }
}

void WeightedGraph::addRandomEdges(double addedEdgesProportion) {
    uint n = (double) getNumNodes() * addedEdgesProportion;
    for (uint i = 0; i <= n; i++) {
        addRandomEdge();
    }
    initConnectedComponents();
}

void WeightedGraph::removeRandomEdges(double removedEdgesProportion) {
    uint n = (double) getNumNodes() * removedEdgesProportion;
    for (uint i = 0; i <= n; i++) {
        removeRandomEdge();
    }
    initConnectedComponents();
}

void WeightedGraph::rewireRandomEdges(double rewiredEdgesProportion) {
    uint n = (double) getNumNodes() * rewiredEdgesProportion;
    for (uint i = 0; i <= n; i++) {
        addRandomEdge();
        removeRandomEdge();
    }
    initConnectedComponents();
}

