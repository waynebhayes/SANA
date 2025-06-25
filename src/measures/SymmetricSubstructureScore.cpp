#include <vector>
#include "SymmetricSubstructureScore.hpp"

// Forward Declaration
const Graph *SymmetricSubstructureScore::G1, *SymmetricSubstructureScore::G2;

SymmetricSubstructureScore::SymmetricSubstructureScore(const Graph* G1, const Graph* G2) : Measure(G1, G2, "s3") {
    assert(SymmetricSubstructureScore::G1==NULL);
    assert(SymmetricSubstructureScore::G2==NULL);
    SymmetricSubstructureScore::G1=G1;
    SymmetricSubstructureScore::G2=G2;
}

SymmetricSubstructureScore::~SymmetricSubstructureScore() 
{}

double SymmetricSubstructureScore::eval(const Alignment& A) {
    double aligEdges = A.computeNumAlignedEdges(*G1, *G2);
    return aligEdges / 
        (G1->getNumEdges() + G2->numEdgesInNodeInducedSubgraph(A.asVector()) - aligEdges);
}

double SymmetricSubstructureScore::getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2) {
    // Unimplemented
    return -.1;
}

double SymmetricSubstructureScore::getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    // Unimplemented
    return -.1;
}

double SymmetricSubstructureScore::getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    // Unimplemented
    return -.1;
}

double SymmetricSubstructureScore::getSum(const Alignment &A) {
    // Unimplemented
    return -.1;
}

