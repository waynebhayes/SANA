#include "Graph.hpp"
#include "Utils.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <sstream>

using namespace std;

string Graph::getName() const {
    return Graph::name;
}

void Graph::setName(string name) {
    Graph::name = name;
}

unsigned int Graph::GetNumNodes() const {
	return Graph::numNodes;
}

void Graph::SetNumNodes(const unsigned int &numNodes) {
    adjLists.resize(numNodes);
}

unsigned int Graph::GetNumEdges() const {
    return Graph::numEdges;
}

int Graph::RandomNode() {
    return Utils::RandInt(0,GetNumNodes()-1);
}

void Graph::AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight) throw(GraphInvalidIndexError) {
    if (node1 >= numNodes || node2 >= numNodes)
        throw GraphInvalidIndexError("Invalid node index passed into AddEdge");

    adjLists[node1].push_back(node2);
    adjLists[node2].push_back(node1);
    ++numEdges;
}

void Graph::RemoveEdge(const unsigned int &node1, const unsigned int &node2) throw(GraphInvalidIndexError) {
    if (node1 >= numNodes || node2 >= numNodes)
        throw GraphInvalidIndexError("Invalid node index passed into RemoveEdge");

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


void Graph::AddRandomEdge() {
    int node1 = 0, node2 = 0;
    while(node1 == node2 or adjLists[node1][node2]) {
        node1 = RandomNode();
        node2 = RandomNode();
    }
    AddEdge(node1,node2);
}

void Graph::RemoveRandomEdge() {
    int node1 = 0, node2 = 0;
    while(node1 == node2 or adjLists[node1][node2]) {
        node1 = RandomNode();
        node2 = RandomNode();
    }
    RemoveEdge(node1,node2);
}

uint Graph::getGeneCount() const {
    return geneCount;
}

void Graph::SetNumNodes(const unsigned int &numNodes) {
    adjLists.resize(numNodes);
    nodesName.resize(numNodes);
}

void Graph::SetNodeName(const unsigned int &nodeIndex, const string &name) throw(GraphInvalidIndexError) {
    if (nodeIndex >= numNodes)
        throw GraphInvalidIndexError("Invalid node index passed into SetNodeName");
    nodesName[nodeIndex] = name;
}

string Graph::GetNodeName(const unsigned int &nodeIndex) const throw(GraphInvalidIndexError) {
     if (nodeIndex >= numNodes)
         throw GraphInvalidIndexError("Invalid node index passed into GetNodeName");
     return nodesName[nodeIndex];
}

void Graph::ClearGraph() {
    adjLists.clear();
    numNodes = numEdges = 0;
}

void Graph::setGeneCount(uint geneCount) {
    Graph::geneCount = geneCount;
}

uint Graph::getMiRNACount() const {
    return miRNACount;
}

void Graph::setMiRNACount(uint miRNACount) {
    Graph::miRNACount = miRNACount;
}

int Graph::getUnlockedGeneCount() const {
    return unlockedGeneCount;
}

void Graph::setUnlockedGeneCount(int unlockedGeneCount) {
    Graph::unlockedGeneCount = unlockedGeneCount;
}

int Graph::getUnlockedmiRNACount() const {
    return unlockedmiRNACount;
}

void Graph::setUnlockedmiRNACount(int unlockedmiRNACount) {
    Graph::unlockedmiRNACount = unlockedmiRNACount;
}

const vector<string> &Graph::getNodeTypes() const {
    return nodeTypes;
}

void Graph::addNodeType(const string &nodeType) {
    nodeTypes.push_back(nodeType);
}


