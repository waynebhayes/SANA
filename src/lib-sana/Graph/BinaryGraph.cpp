#include "BinaryGraph.hpp"


void BinaryGraph::AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int& weight) {
    Graph::AddEdge(node1, node2);
    if(adjacencyMatrix[node1][node2] == false && node1 != node2)
        adjacencyMatrix[node1][node2] = adjacencyMatrix[node2][node1] = true;
}

void BinaryGraph::RemoveEdge(const unsigned int &node1, const unsigned int &node2) {
    Graph::RemoveEdge(node1, node2);
    adjacencyMatrix[node1][node2] = adjacencyMatrix[node2][node1] = false;
}

void BinaryGraph::SetNumNodes(const unsigned int &numNodes) {
    Graph::SetNumNodes(numNodes);
    adjacencyMatrix.resize(numNodes);
    for (int i = 0; i < numNodes; i++)
        adjacencyMatrix[i].resize(numNodes);
}

void BinaryGraph::ClearGraph() {
    Graph::ClearGraph();
    adjacencyMatrix.clear();
}
