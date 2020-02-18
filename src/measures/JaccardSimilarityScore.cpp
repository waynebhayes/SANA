#include <vector>
#include "JaccardSimilarityScore.hpp"

JaccardSimilarityScore::JaccardSimilarityScore(Graph* G1, Graph* G2) : Measure(G1, G2, "js") {
}

JaccardSimilarityScore::~JaccardSimilarityScore() {
}

double JaccardSimilarityScore::eval(const Alignment& A) {
    double jsSum = 0;
    uint n1 = G1->getNumNodes();
    vector<vector<uint>> G1AdjLists = G1->getAdjLists();
    Matrix<MATRIX_UNIT> G2Matrix = G2->getMatrix();

    for (uint i = 0; i < n1; i++){
        // dont need to check if i is aligned coz g1 is smaller and always will be alinged
        uint iAlignedTo = A[i]; //find the node i is mapped to
        uint iAlignedEdges = 0;
        vector<uint> iNeighbours = G1AdjLists[i];
        uint iTotalEdges = iNeighbours.size();
        for (uint j = 0; j < iTotalEdges; j++){
            uint neighbour = iNeighbours[j];
            uint neighbourAlignedTo = A[neighbour]; //find the node neighbour is mapped to
            if(G2Matrix[iAlignedTo][neighbourAlignedTo] == true){
                iAlignedEdges += 1;
            }
        }
        jsSum += (iAlignedEdges/iTotalEdges);
    }

    return jsSum;
}

vector<uint> JaccardSimilarityScore::getAlignedByNode(const Alignment& A){
    uint n1 = G1->getNumNodes();
    vector<vector<uint>> G1AdjLists = G1->getAdjLists();
    Matrix<MATRIX_UNIT> G2Matrix = G2->getMatrix();
    vector<uint> alignedByNode;

    for (uint i = 0; i < n1; i++){
        // dont need to check if i is aligned coz g1 is smaller and always will be alinged
        uint iAlignedTo = A[i]; //find the node i is mapped to
        uint iAlignedEdges = 0;
        vector<uint> iNeighbours = G1AdjLists[i];
        uint iTotalEdges = iNeighbours.size();
        for (uint j = 0; j < iTotalEdges; j++){
            uint neighbour = iNeighbours[j];
            uint neighbourAlignedTo = A[neighbour]; //find the node neighbour is mapped to
            if(G2Matrix[iAlignedTo][neighbourAlignedTo] == true){
                iAlignedEdges += 1;
            }
        }
        alignedByNode.push_back(iAlignedEdges);
    }

    return alignedByNode;
}
