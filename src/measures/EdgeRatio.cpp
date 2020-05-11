#include "EdgeRatio.hpp"
#include <vector>
#include <array>

EdgeRatio::EdgeRatio(const Graph* G1, const Graph* G2): Measure(G1, G2, "er") {}
EdgeRatio::~EdgeRatio() {}

double EdgeRatio::eval(const Alignment& A) {
#ifndef FLOAT_WEIGHTS
    return kErrorScore;
#else
    uint n = G1->getNumNodes();
    double edgeRatioSum = getEdgeRatioSum(G1, G2, A);
    uint pairsCount = (n * (n+1))/2;
    return adjustSumToTargetScore(edgeRatioSum, pairsCount);
#endif
}


double EdgeRatio::getEdgeRatioSum(const Graph *G1, const Graph *G2, const Alignment &A) {
#ifndef FLOAT_WEIGHTS
    return 0;
#else
    double edgeRatioSum = 0;
    double c = 0;
    for (const auto& edge : *(G1->getEdgeList())) {
      uint node1 = edge[0], node2 = edge[1];
      double r = getRatio(G1->getEdgeWeight(node1, node2), G2->getEdgeWeight(A[node1], A[node2]));
      double y = r - c;
      double t = edgeRatioSum + y;
      c = (t - edgeRatioSum) - y;
      edgeRatioSum = t;
    }
    return edgeRatioSum;
#endif
}

double EdgeRatio::adjustSumToTargetScore(double edgeRatioSum, uint pairsCount) {
    double mean = edgeRatioSum / pairsCount;
    return mean;
}

double EdgeRatio::getRatio(double w1, double w2) {
    if (w1 == 0 and w2 == 0) return 1;
    double r = (abs(w1) < abs(w2) ? w1/w2 : w2/w1);
    //At this point, r is in [-1,1], but we want it in [0,1], so add 1 and divide by 2
    r = (r+1)/2;
    assert(r >= 0 and r <= 1);
    return r;
}
