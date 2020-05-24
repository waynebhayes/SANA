#include "EdgeCorrectness.hpp"
#include <string>
#include <vector>

EdgeCorrectness::EdgeCorrectness(const Graph* G1, const Graph* G2) : Measure(G1, G2, "ec") {
}

EdgeCorrectness::~EdgeCorrectness() {
}

double EdgeCorrectness::eval(const Alignment& A) {
    return (double) A.numAlignedEdges(*G1, *G2)/G1->getNumEdges();
}
