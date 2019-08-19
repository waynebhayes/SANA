#include "WeightedGraph.hpp"

void WeightedGraph::AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int& weight) {
    Graph::AddEdge(node1, node2, weight);
    adjacencyMatrix[node1][node2] = adjacencyMatrix[node2][node1] = weight;
}

void WeightedGraph::RemoveEdge(const unsigned int &node1, const unsigned int &node2) {
    Graph::RemoveEdge(node1, node2);
    adjacencyMatrix[node1][node2] = adjacencyMatrix[node2][node1] = 0;
}

void WeightedGraph::SetNumNodes(const unsigned int &numNodes) {
    Graph::SetNumNodes(numNodes);
    adjacencyMatrix.resize(numNodes);
    for (int i = 0; i < numNodes; i++) 
        adjacencyMatrix[i].resize(numNodes);
}

void WeightedGraph::ClearGraph() {
    Graph::ClearGraph();
    adjacencyMatrix.clear();
}
