#include "SquaredEdgeScore.hpp"
#include <cmath>

SquaredEdgeScore::SquaredEdgeScore(Graph* G1, Graph* G2) : Measure(G1, G2, "ses") {
}

SquaredEdgeScore::~SquaredEdgeScore() {
}

double SquaredEdgeScore::eval(const Alignment& A) {
    return (double) A.numSquaredAlignedEdges(*G1, *G2);
}
