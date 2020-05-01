#include "EdgeDifference.hpp"
#include <vector>

EdgeDifference::EdgeDifference(const Graph* G1, const Graph* G2): 
    Measure(G1, G2, "ed") {}

EdgeDifference::~EdgeDifference() {
}

double EdgeDifference::eval(const Alignment& A) {
    uint n1 = G1->getNumNodes();
    uint pairsCount = (n1 * (n1 + 1)) / 2;
    double edgeDifferenceSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, A);
    return EdgeDifference::adjustSumToTargetScore(edgeDifferenceSum, pairsCount);
}

double EdgeDifference::getEdgeDifferenceSum(const Graph* G1, const Graph* G2, const Alignment &A) {
    double edgeDifferenceSum = 0;
    double c = 0; //use descriptive name please
    for (const auto& edge : *(G1->getEdgeList())) {
       uint node1 = edge[0], node2 = edge[1];
       double y = abs(G1->edgeWeight(node1,node2) - G2->edgeWeight(A[node1],A[node2])) - c;
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
