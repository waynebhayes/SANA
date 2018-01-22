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
    return this->name;
}

void Graph::setName(string name) {
    this->name = name;
}

unsigned int Graph::GetNumNodes() const {
	return numNodes;
}

void Graph::SetNumNodes(const unsigned int &numNodes) {
    adjLists.resize(numNodes);
}

unsigned int Graph::GetNumEdges() const {
    return numEdges;
}

int Graph::RandomNode() {
    return Utils::RandInt(0,GetNumNodes()-1);
}

void Graph::AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight) {
    adjLists[node1].push_back(node2);
    adjLists[node2].push_back(node1);
    ++numEdges;
}

void Graph::RemoveEdge(const unsigned int &node1, const unsigned int &node2) { 
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