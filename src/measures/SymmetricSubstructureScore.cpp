#include <vector>
#include "SymmetricSubstructureScore.hpp"

SymmetricSubstructureScore::SymmetricSubstructureScore(Graph* G1, Graph* G2) : Measure(G1, G2, "s3") {
}

SymmetricSubstructureScore::~SymmetricSubstructureScore() {
}

double SymmetricSubstructureScore::eval(const Alignment& A) {
    double aligEdges = A.numAlignedEdges(*G1, *G2);
    return aligEdges / 
        (G1->getNumEdges() + G2->numNodeInducedSubgraphEdges(A.getMapping()) - aligEdges);
}
