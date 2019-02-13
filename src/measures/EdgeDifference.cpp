#include "EdgeDifference.hpp"
#include <vector>

EdgeDifference::EdgeDifference(Graph* g1, Graph* g2): 
    Measure(G1, G2, "ed"),
    G1(g1),
    G2(g2),
    G1NodesCount(g1->getNumNodes()) {
}

EdgeDifference::~EdgeDifference() {
}

double EdgeDifference::eval(const Alignment& A) {
    double edgeDifferenceSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, A);
    uint pairsCount = (G1NodesCount * (G1NodesCount + 1)) / 2;
    return EdgeDifference::adjustSumToTargetScore(edgeDifferenceSum, pairsCount);
}


double EdgeDifference::getEdgeDifferenceSum(Graph *G1, Graph *G2, const Alignment &A) {
    Matrix<float>& G1FloatWeights = G1->getFloatWeights();
    Matrix<float>& G2FloatWeights = G2->getFloatWeights();
/*
    double edgeDifferenceSum = 0;
    edgeDifferenceSum = abs(G1FloatWeights[0][0] - G2FloatWeights[A[0]][A[0]]);
    uint G1NodesCount = G1->getNumNodes();
    double compensation = 0;
    for (uint node1 = 0; node1 < G1NodesCount; ++node1) {
        for (uint node2 = node1; node2 < G1NodesCount; ++node2) {
            if (node2 == 0) {continue;}
            double y = abs(G1FloatWeights[node1][node2] - G2FloatWeights[A[node1]][A[node2]]) - compensation;
            double t = edgeDifferenceSum + y;
            compensation = (t - edgeDifferenceSum) - y;
            edgeDifferenceSum = t;
            //edgeDifferenceSum += abs(G1FloatWeights[node1][node2] - G2FloatWeights[A[node1]][A[node2]]);
        }
    }

    return edgeDifferenceSum;
*/  
    uint G1NodesCount = G1->getNumNodes();
    double edgeDifferenceSum = 0;
    for (uint node1 = 0; node1 < G1NodesCount; ++node1) {
       for (uint node2 = node1; node2 < G1NodesCount; ++node2) { 
           edgeDifferenceSum += abs(G1FloatWeights[node1][node2] - G2FloatWeights[A[node1]][A[node2]]);
       }
    }

    return edgeDifferenceSum;
}

double EdgeDifference::adjustSumToTargetScore(double edgeDifferenceSum, uint pairsCount) {
    double mean = edgeDifferenceSum / pairsCount;
    return 1 - mean / 2;
}
