#include "EdgeCorrectness.hpp"
#include <vector>

EdgeCorrectness::EdgeCorrectness(Graph* G1, Graph* G2) : Measure(G1, G2, "ec") {
}

EdgeCorrectness::~EdgeCorrectness() {
}

double EdgeCorrectness::eval(const Alignment& A) {
    return (double) A.numAlignedEdges(*G1, *G2)/G1->getNumEdges();
}
