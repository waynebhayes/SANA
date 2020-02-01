#include "EdgeRatio.hpp"
#include <vector>

static double getRatio(double w1, double w2) {
    double r;
    if(w1==0 && w2==0)r=1;
    else if(abs(w1)<abs(w2))r=w1/w2;
    else r=w2/w1;
    // At this point, r can be in [-1,1], but we want it [0,1], so add 1 and divide by 2
    r = (r+1)/2;
    assert(r>=0 && r<=1);
    return r;
}

EdgeRatio::EdgeRatio(Graph* g1, Graph* g2): 
    Measure(g1, g2, "er"),
    G1NodesCount(g1->getNumNodes()) {
}

EdgeRatio::~EdgeRatio() {
}

double EdgeRatio::eval(const Alignment& A) {
    if (!G1->hasFloatWeight() || !G2->hasFloatWeight()) {
        return kErrorScore;
    }

    double edgeRatioSum = EdgeRatio::getEdgeRatioSum(G1, G2, A);
    uint pairsCount = (G1NodesCount * (G1NodesCount + 1)) / 2;
    return EdgeRatio::adjustSumToTargetScore(edgeRatioSum, pairsCount);
}


double EdgeRatio::getEdgeRatioSum(Graph *G1, Graph *G2, const Alignment &A) {
    Matrix<float>& G1FloatWeights = G1->getFloatWeights();
    Matrix<float>& G2FloatWeights = G2->getFloatWeights();

    double edgeRatioSum = 0;
    double c = 0;
    const vector<vector<uint>> &edgeLists = G1->getEdgeList();
    for (uint i = 0; i < edgeLists.size(); ++i) {
      uint node1 = edgeLists[i][0], node2 = edgeLists[i][1];
      double r = getRatio(G1FloatWeights[node1][node2], G2FloatWeights[A[node1]][A[node2]]);
      double y = r - c;
      double t = edgeRatioSum + y;
      c = (t - edgeRatioSum) - y;
      edgeRatioSum = t;
    }
    

    return edgeRatioSum;
}

double EdgeRatio::adjustSumToTargetScore(double edgeRatioSum, uint pairsCount) {
    double mean = edgeRatioSum / pairsCount;
    return mean;
}
