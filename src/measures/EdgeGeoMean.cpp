#include "EdgeGeoMean.hpp"
#include <vector>

const Graph *EdgeGeoMean::G1, *EdgeGeoMean::G2;
double EdgeGeoMean::denominator;

EdgeGeoMean::EdgeGeoMean(const Graph* G1, const Graph* G2): Measure(G1, G2, "ed") {
    EdgeGeoMean::G1=G1;
    EdgeGeoMean::G2=G2;
    denominator = computeDenom(G1,G2);
}


EdgeGeoMean::~EdgeGeoMean() {
}

double EdgeGeoMean::eval(const Alignment& A) {
    return getEdgeGeoMeanSum(G1, G2, A)/denominator;
}

double EdgeGeoMean::getEdgeScore(double w1, double w2) {
    double sgn = w1*w2>=0 ? 1:-1;
    return sgn * sqrt(abs(w1*w2));
}


static int CmpEdge(EDGE_T w1, EDGE_T w2) {return (w1>w2);}

double EdgeGeoMean::computeDenom(const Graph* G1, const Graph* G2) {
    vector<EDGE_T> W1, W2;
    for (const auto& edge : *(G1->getEdgeList())) W1.push_back(G1->getEdgeWeight(edge[0],edge[1]));
    for (const auto& edge : *(G2->getEdgeList())) W2.push_back(G1->getEdgeWeight(edge[0],edge[1]));
    sort(W1.begin(), W1.end(), CmpEdge);
    sort(W2.begin(), W2.end(), CmpEdge);
    double sum = 0.0;
    for(int i=0; i<min(G1->getNumNodes(), G2->getNumNodes()); i++) sum+= getEdgeScore(W1[i], W2[i]);
    return sum;
}

double EdgeGeoMean::getEdgeGeoMeanSum(const Graph* G1, const Graph* G2, const Alignment &A) {
    double sum = 0;
    double c = 0; //use descriptive name please
    for (const auto& edge : *(G1->getEdgeList())) {
       uint node1 = edge[0], node2 = edge[1];
       double y = getEdgeScore(G1->getEdgeWeight(node1,node2), G2->getEdgeWeight(A[node1],A[node2])) - c;
       double t = sum + y;
       c = (t - sum) - y;
       sum = t;
    }
    return sum;
}

double EdgeGeoMean::getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    double val = computeIncChangeOp(peg, oldHole, newHole, A);
    assert(val == val); // will fail if NaN
    return val;
}

double EdgeGeoMean::computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A) {
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    for (uint nbr : *(G1->getAdjList(peg))) {
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

double EdgeGeoMean::getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    return computeIncSwapOp(peg1, peg2, hole1, hole2, A);
}

/* We swap the mapping of two nodes peg1 and peg2
 * We can first handle peg1, then do the same with peg2
 * Subtract old edge difference with edge (peg1, hole1)
 * Add new edge difference with edge (peg1, hole2) */
double EdgeGeoMean::computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A) {
    if (peg1 == peg2) return 0;
    // Handle peg1
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    for (uint nbr : *(G1->getAdjList(peg1))) {
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
    // Handle peg2
    for (uint nbr : *(G1->getAdjList(peg2))) {
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


