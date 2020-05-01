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
    const vector<array<uint, 2>>* edgeList = G1->getEdgeList();
    for (const auto& edge : *edgeList) {
      uint node1 = edge[0], node2 = edge[1];
      double r = getRatio(G1->edgeWeight(node1, node2), G2->edgeWeight(A[node1], A[node2]));
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
    double r;
    if (w1==0 && w2==0) r = 1;
    else if (abs(w1)<abs(w2)) r = w1/w2;
    else r = w2/w1;
    // At this point, r can be in [-1,1], but we want it [0,1], so add 1 and divide by 2
    r = (r+1)/2;
    assert(r>=0 && r<=1);
    return r;
}
