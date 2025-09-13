#include "EdgeDifference.hpp"
#include <vector>

const Graph *EdgeDifference::G1, *EdgeDifference::G2;
EdgeDifference::EdgeDifference(const Graph* G1, const Graph* G2): Measure(G1, G2, "ed") {
    EdgeDifference::G1=G1;
    EdgeDifference::G2=G2;
}

EdgeDifference::~EdgeDifference() {
}

double EdgeDifference::eval(const Alignment& A) {
    double edgeDifferenceSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, A);
    return EdgeDifference::adjustSumToTargetScore(G1, G2, edgeDifferenceSum);
}

double EdgeDifference::getEdgeDifferenceSum(const Graph* G1, const Graph* G2, const Alignment &A) {
    double edgeDifferenceSum = 0;
    double c = 0; //use descriptive name please
    for (const auto& edge : *(G1->getEdgeList())) {
       uint node1 = edge[0], node2 = edge[1];
       double y = abs(G1->getEdgeWeight(node1,node2) - G2->getEdgeWeight(A[node1],A[node2])) - c;
       double t = edgeDifferenceSum + y;
       c = (t - edgeDifferenceSum) - y;
       edgeDifferenceSum = t;
    }
    return edgeDifferenceSum;
}

double EdgeDifference::adjustSumToTargetScore(const Graph *G1, const Graph *G2, const double edgeDifferenceSum) {
    uint m1=  G1->getNumEdges(), m2=G2->getNumEdges();
    double mean = edgeDifferenceSum / min(m1,m2);
    return 1 - mean / 2;
}

double EdgeDifference::getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    double val = computeIncChangeOp(peg, oldHole, newHole, A);
    assert(val == val); // will fail if NaN
    return val;
}

double EdgeDifference::computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A) {
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    unique_ptr<vector<unsigned>> G1pegAdj = G1->getAdjList(peg);
    for (uint nbr : *G1pegAdj) {
        double y = -abs(G1->getEdgeWeight(peg, nbr) - G2->getEdgeWeight(oldHole, A[nbr])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint nbrHole = nbr == peg ? newHole : A[nbr];
        y = +abs(G1->getEdgeWeight(peg, nbr) - G2->getEdgeWeight(newHole, nbrHole)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    return edgeDifferenceIncDiff;
}

double EdgeDifference::getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    return computeIncSwapOp(peg1, peg2, hole1, hole2, A);
}

/* We swap the mapping of two nodes peg1 and peg2
 * We can first handle peg1, then do the same with peg2
 * Subtract old edge difference with edge (peg1, hole1)
 * Add new edge difference with edge (peg1, hole2) */
double EdgeDifference::computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A) {
    if (peg1 == peg2) return 0;
    // Handle peg1
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    unique_ptr<vector<unsigned>> G1peg1Adj = G1->getAdjList(peg1);
    for (uint nbr : *G1peg1Adj) {
        double y = -abs(G1->getEdgeWeight(peg1, nbr) - G2->getEdgeWeight(hole1, A[nbr])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        // Determine the new target hole for nbr
        uint nbrHole = 0;
        if (nbr == peg1) nbrHole = hole2;
        else if (nbr == peg2) nbrHole = hole1;
        else nbrHole = A[nbr];

        y = +abs(G1->getEdgeWeight(peg1, nbr) - G2->getEdgeWeight(hole2, nbrHole)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    unique_ptr<vector<unsigned>> G1peg2Adj = G1->getAdjList(peg2);
    for (uint nbr : *G1peg2Adj) {
        if (nbr == peg1) continue;
        double y = -abs(G1->getEdgeWeight(peg2, nbr) - G2->getEdgeWeight(hole2, A[nbr])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint nbrHole = (nbr == peg2 ? hole1 : A[nbr]);
        y = +abs(G1->getEdgeWeight(peg2, nbr) - G2->getEdgeWeight(hole1, nbrHole)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    return edgeDifferenceIncDiff;
}


