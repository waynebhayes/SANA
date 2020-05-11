#include "JaccardSimilarityScore.hpp"
#include <vector>

JaccardSimilarityScore::JaccardSimilarityScore(const Graph* G1, const Graph* G2): Measure(G1, G2, "js") {}

JaccardSimilarityScore::~JaccardSimilarityScore() {}

double JaccardSimilarityScore::eval(const Alignment& A) {
    double jsSum = 0;
    const vector<vector<uint>>* G1AdjLists = G1->getAdjLists();
    vector<uint> alignedByNode = JaccardSimilarityScore::getAlignedByNode(G1, G2, A);

    for (uint i = 0; i< G1->getNumNodes(); i++){
        vector<uint> iNeighbours = (*G1AdjLists)[i];
        uint iTotalEdges = iNeighbours.size();
        jsSum += (alignedByNode[i]/(double)iTotalEdges);
    }
    return jsSum;
}

vector<uint> JaccardSimilarityScore::getAlignedByNode(const Graph *G1, const Graph *G2, const Alignment& A){
    const vector<vector<uint>>* G1AdjLists = G1->getAdjLists();
    vector<uint> alignedByNode;
    for (uint i = 0; i < G1->getNumNodes(); i++){
        // dont need to check if i is aligned because g1 is smaller and always will be alinged
        uint iAlignedTo = A[i]; //find the node i is mapped to
        uint iAlignedEdges = 0;
        vector<uint> iNeighbours = (*G1AdjLists)[i];
        uint iTotalEdges = iNeighbours.size();
        for (uint j = 0; j < iTotalEdges; j++){
            uint neighbour = iNeighbours[j];
            uint neighbourAlignedTo = A[neighbour]; //find the node neighbour is mapped to
            iAlignedEdges += G2->getEdgeWeight(iAlignedTo,neighbourAlignedTo);
        }
        alignedByNode.push_back(iAlignedEdges);
    }
    return alignedByNode;
}
