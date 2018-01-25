#include "Graph.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <sstream>
#include "Utils.hpp"

using namespace std;

unsigned int Graph::GetNumNodes() const {
	return numNodes;
}

unsigned int Graph::GetNumEdges() const {
    return numEdges;
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
