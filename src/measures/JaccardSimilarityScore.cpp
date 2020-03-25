#include "JaccardSimilarityScore.hpp"
#include <vector>

JaccardSimilarityScore::JaccardSimilarityScore(Graph* G1, Graph* G2) : Measure(G1, G2, "js"), G1NodesCount(G1->getNumNodes()) {
}

JaccardSimilarityScore::~JaccardSimilarityScore() {
}

double JaccardSimilarityScore::eval(const Alignment& A) {
    double jsSum = 0;
    vector<vector<uint>> G1AdjLists;
    G1->getAdjLists(G1AdjLists);
    Matrix<MATRIX_UNIT> G2Matrix;
    G2->getMatrix(G2Matrix);

    vector<uint> alignedByNode = JaccardSimilarityScore::getAlignedByNode(G1, G2, A);

    for (uint i = 0; i< G1NodesCount; i++){
        vector<uint> iNeighbours = G1AdjLists[i];
        uint iTotalEdges = iNeighbours.size();
        jsSum += (alignedByNode[i]/(double)iTotalEdges);
    }
    return jsSum;
}

vector<uint> JaccardSimilarityScore::getAlignedByNode(Graph *G1, Graph *G2, const Alignment& A){
    vector<vector<uint>> G1AdjLists;
    G1->getAdjLists(G1AdjLists);
    Matrix<MATRIX_UNIT> G2Matrix;
    G2->getMatrix(G2Matrix);
    vector<uint> alignedByNode;
    uint G1NodesCount = G1->getNumNodes();

    for (uint i = 0; i < G1NodesCount; i++){
        // dont need to check if i is aligned coz g1 is smaller and always will be alinged
        uint iAlignedTo = A[i]; //find the node i is mapped to
        uint iAlignedEdges = 0;
        vector<uint> iNeighbours = G1AdjLists[i];
        uint iTotalEdges = iNeighbours.size();
        for (uint j = 0; j < iTotalEdges; j++){
            uint neighbour = iNeighbours[j];
            uint neighbourAlignedTo = A[neighbour]; //find the node neighbour is mapped to
            iAlignedEdges += G2Matrix[iAlignedTo][neighbourAlignedTo];
        }
        alignedByNode.push_back(iAlignedEdges);
    }

    return alignedByNode;
}
