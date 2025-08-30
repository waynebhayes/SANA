#include "Misc.hpp"
#include "EdgeCorrectness.hpp"
#include <string>
#include <vector>

const Graph* EdgeCorrectness::G1 = nullptr;
const Graph* EdgeCorrectness::G2 = nullptr;
EdgeCorrectness::EdgeCorrectness(const Graph* G1, const Graph* G2, int graphNum): Measure(G1, G2, "ec") {
    EdgeCorrectness::G1=G1;
    EdgeCorrectness::G2=G2;
    switch (graphNum) {
        case 1: denominator = EdgeCorrectness::G1->getNumEdges(); break;
        case 2: denominator = EdgeCorrectness::G2->getNumEdges(); break;
        default: Fatal("unknown graphNum %d in EdgeCorrectness::EdgeCorrectness", graphNum);
    }
}

EdgeCorrectness::~EdgeCorrectness() {
}

double EdgeCorrectness::eval(const Alignment& A) {
    return static_cast<double>(A.computeNumAlignedEdges(*G1, *G2)) / denominator;
}
