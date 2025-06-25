#include "Misc.hpp"
#include "EdgeCorrectness.hpp"
#include <string>
#include <vector>

// Forward Declaration
const Graph *EdgeCorrectness::G1, *EdgeCorrectness::G2;
double EdgeCorrectness::denominator;

// Note: EC assumes that G2 is the denominator graph.
EdgeCorrectness::EdgeCorrectness(const Graph* G1, const Graph* G2) : Measure(G1, G2, "ec") {
    assert(EdgeCorrectness::denominator==0);
    assert(EdgeCorrectness::G1==NULL);
    assert(EdgeCorrectness::G2==NULL);
    EdgeCorrectness::G1=G1;
    EdgeCorrectness::G2=G2;
    EdgeCorrectness::denominator=G2->getNumEdges();
}

EdgeCorrectness::~EdgeCorrectness() 
{}

double EdgeCorrectness::eval(const Alignment& A) {
    return (double) A.computeNumAlignedEdges(*G1, *G2) / EdgeCorrectness::denominator;
}

double EdgeCorrectness::getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2) {
    // Unimplemented
    return -.1;
}

double EdgeCorrectness::getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    // Unimplemented
    return -.1;
}

double EdgeCorrectness::getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    // Unimplemented
    return -.1;
}

double EdgeCorrectness::getSum(const Alignment &A) {
    // Unimplemented
    return -.1;
}

