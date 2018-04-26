//
// Created by taylor on 4/26/18.
//

#include "SymmetricSubstructureScore.hpp"
#include <vector>

SymmetricSubstructureScore::SymmetricSubstructureScore(Graph* G1, Graph* G2) : Measure(G1, G2, "s3") {
}

SymmetricSubstructureScore::~SymmetricSubstructureScore() {
}

double SymmetricSubstructureScore::eval(const Alignment& A) {
    double aligEdges = A.numAlignedEdges(*G1, *G2);
    return aligEdges /
           (G1->GetNumEdges() + G2->NumNodeInducedSubgraphEdges(A.getMapping()) - aligEdges);
}