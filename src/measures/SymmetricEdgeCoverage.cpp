#include "SymmetricEdgeCoverage.hpp"
#include <vector>
#include <iostream>
SymmetricEdgeCoverage::SymmetricEdgeCoverage(Graph* G1, Graph* G2) : Measure(G1, G2, "sec") {
}

SymmetricEdgeCoverage::~SymmetricEdgeCoverage() {
}

double SymmetricEdgeCoverage::eval(const Alignment& A) {
    double result = (double) A.numAlignedEdges(*G1, *G2)/G1->getNumEdges();
    result += (double) A.numAlignedEdges(*G1, *G2)/G2->getNumEdges();
    result = result/2;

    return result;
}
