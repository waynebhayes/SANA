#include <vector>
#include <cmath>
#include "LargestCommonConnectedSubgraph.hpp"

LargestCommonConnectedSubgraph::LargestCommonConnectedSubgraph(Graph* G1, Graph* G2) : Measure(G1, G2, "lccs") {
}

LargestCommonConnectedSubgraph::~LargestCommonConnectedSubgraph() {}

double LargestCommonConnectedSubgraph::eval(const Alignment& A) {
    Graph CS = G1->graphIntersection(*G2, *(A.getVector()));
    cerr<<'i';
    vector<uint> LCCSNodes = (*CS.getConnectedComponents())[0]; //largest CC
    cerr<<'i';
    uint n = LCCSNodes.size();
    double N = (double) n/G1->getNumNodes();
    if (not USE_MAGNA_DEFINITION) return N;

    // To Get the indexes of the common subgraph in G2
    vector<uint> LCCSNodesG2;
    LCCSNodesG2.reserve(LCCSNodes.size());
    for(uint node: LCCSNodes) LCCSNodesG2.push_back(A[node]);

    cerr<<'i';
    Graph G1InducedSubgraph = G1->nodeInducedSubgraph(LCCSNodes);
    cerr<<'i';
    Graph G2InducedSubgraph = G2->nodeInducedSubgraph(LCCSNodesG2);
    cerr<<'i';
    double E1 = (double) G1InducedSubgraph.getNumEdges()/G1->getNumEdges();
    double E2 = (double) G2InducedSubgraph.getNumEdges()/G2->getNumEdges();
    double E = min(E1,E2);
    return sqrt(E*N);
}
