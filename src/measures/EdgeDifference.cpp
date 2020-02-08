#include "EdgeDifference.hpp"
#include <vector>

EdgeDifference::EdgeDifference(Graph* g1, Graph* g2): 
    Measure(g1, g2, "ed"),
    G1NodesCount(g1->getNumNodes()) {
}

EdgeDifference::~EdgeDifference() {
}

double EdgeDifference::eval(const Alignment& A) {
    if (!G1->hasFloatWeight() || !G2->hasFloatWeight()) {
        return kErrorScore;
    }

    double edgeDifferenceSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, A);
    uint pairsCount = (G1NodesCount * (G1NodesCount + 1)) / 2;
    return EdgeDifference::adjustSumToTargetScore(edgeDifferenceSum, pairsCount);
}

double EdgeDifference::getEdgeDifferenceSum(Graph *G1, Graph *G2, const Alignment &A) {
    Matrix<float>& G1FloatWeights = G1->getFloatWeights();
    Matrix<float>& G2FloatWeights = G2->getFloatWeights();

    double edgeDifferenceSum = 0;
    double c = 0;

    const vector<vector<uint>> &edgeLists = G1->getEdgeList();
    for (uint i = 0; i < edgeLists.size(); ++i) {
       uint node1 = edgeLists[i][0], node2 = edgeLists[i][1];
       double y = abs(G1FloatWeights[node1][node2] - G2FloatWeights[A[node1]][A[node2]]) - c;
       double t = edgeDifferenceSum + y;
       c = (t - edgeDifferenceSum) - y;
       edgeDifferenceSum = t;
    }

    return edgeDifferenceSum;
}

double EdgeDifference::adjustSumToTargetScore(double edgeDifferenceSum, uint pairsCount) {
    double mean = edgeDifferenceSum / pairsCount;
    return 1 - mean / 2;
}
