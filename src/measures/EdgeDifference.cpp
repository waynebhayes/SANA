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
    double sum1 = 0;
    double sum2 = 0;
    for (uint node1 = 0; node1 < G1NodesCount; ++node1) {
       for (uint node2 = node1; node2 < G1NodesCount; ++node2) { 
 //          if (G1FloatWeights[node1][node2] == 0) { cout << node1 << " xx " << node2 << endl; assert(false);}
 //          if (G2FloatWeights[A[node1]][A[node2]] == 0) { cout << A[node1] << " ..  " << A[node2] << endl; assert(false); }
           edgeDifferenceSum += abs(G1FloatWeights[node1][node2] - G2FloatWeights[A[node1]][A[node2]]);
           sum1 += G1FloatWeights[node1][node2];
          sum2 += G2FloatWeights[A[node1]][A[node2]];
       }
    }

    cout << A[10] << endl;
    cout << "@@@@@@@@@@@@" << sum2 << endl;
    return edgeDifferenceSum;
}

double EdgeDifference::adjustSumToTargetScore(double edgeDifferenceSum, uint pairsCount) {
    double mean = edgeDifferenceSum / pairsCount;
    edgeDifferenceSum = 1 - mean / 2;
    return edgeDifferenceSum;
}
