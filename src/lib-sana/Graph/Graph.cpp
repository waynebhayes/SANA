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

void Graph::SetNumNodes(const unsigned int &numNodes) {
    adjLists.resize(numNodes);
}
