#include "EdgeCorrectness.hpp"
#include <string>
#include <vector>

EdgeCorrectness::EdgeCorrectness(const Graph* G1, const Graph* G2, int graphNum) : Measure(G1, G2, "ec") {
    denominatorGraph = graphNum;
}

EdgeCorrectness::~EdgeCorrectness() {
}

double EdgeCorrectness::eval(const Alignment& A) {
    if(denominatorGraph == 1) return (double) A.numAlignedEdges(*G1, *G2)/G1->getNumEdges();
    else if(denominatorGraph == 2) return (double) A.numAlignedEdges(*G1, *G2)/G2->getNumEdges();
}
