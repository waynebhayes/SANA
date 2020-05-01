#include "MultiEdgeCorrectness.hpp"
#include <vector>

MultiEdgeCorrectness::MultiEdgeCorrectness(Graph* G1, Graph* G2) : Measure(G1, G2, "mec") {
}

MultiEdgeCorrectness::~MultiEdgeCorrectness() {
}

double MultiEdgeCorrectness::eval(const Alignment& A) {
#ifdef MULTI_PAIRWISE
    return (double) A.numAlignedEdges(*G1, *G2)/(G2->getTotalEdgeWeight() + G1->getTotalEdgeWeight());
#else
    return 1;
#endif
}
