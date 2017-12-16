#include "WeightedGraph.hpp"

void WeightedGraph::AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int& weight) {
    adjacencyMatrix[node1][node2] = adjacencyMatrix[node2][node1] = weight;
    Graph::AddEdge(node1, node2);
}

void WeightedGraph::RemoveEdge(const unsigned int &node1, const unsigned int &node2) {
    adjacencyMatrix[node1][node2] = adjacencyMatrix[node2][node1] = 0;
    Graph::RemoveEdge(node1, node2);
}