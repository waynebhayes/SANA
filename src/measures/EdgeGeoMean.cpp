#include "EdgeGeoMean.hpp"
#include <vector>

const Graph *EdgeGeoMean::G1, *EdgeGeoMean::G2;
double EdgeGeoMean::denominator;

EdgeGeoMean::EdgeGeoMean(const Graph* G1, const Graph* G2): Measure(G1, G2, "egm") {
    assert(denominator == 0);
    assert(EdgeGeoMean::G1 == NULL);
    assert(EdgeGeoMean::G2 == NULL);
    EdgeGeoMean::G1=G1;
    EdgeGeoMean::G2=G2;
    EdgeGeoMean::denominator = computeDenom(G1,G2);
    std::cerr << "Computed denominator: " << EdgeGeoMean::denominator << std::endl;
}

EdgeGeoMean::~EdgeGeoMean() {
}

double EdgeGeoMean::eval(const Alignment& A) {
    return getEdgeGeoMeanSum(G1, G2, A);
}

double EdgeGeoMean::getEdgeScore(EDGE_T w1, EDGE_T w2) {
    //assert(w1 > 0 && w2 > 0); // for FlyWire, edges are positive
    double sgn = w1*w2>=0 ? 1:-1;
    double numer = sgn * sqrt(abs(w1*w2));
    if(denominator) return numer/denominator;
    else return numer;
}

static bool CmpEdge(EDGE_T w1, EDGE_T w2) { return (w1 > w2); }

double EdgeGeoMean::computeDenom(const Graph* G1, const Graph* G2) {
    vector<EDGE_T> W1, W2;
    for (const auto& edge : *(G1->getEdgeList())) W1.push_back(G1->getEdgeWeight(edge[0],edge[1]));
    for (const auto& edge : *(G2->getEdgeList())) W2.push_back(G2->getEdgeWeight(edge[0],edge[1]));
    sort(W1.begin(), W1.end(), CmpEdge);
    sort(W2.begin(), W2.end(), CmpEdge);
    double sum = 0.0;
    unsigned minSize = min(W1.size(), W2.size());
    for(unsigned i=0; i<minSize; i++) sum += getEdgeScore(W1[i], W2[i]);
    return sum;
}

double EdgeGeoMean::getEdgeGeoMeanSum(const Graph* G1, const Graph* G2, const Alignment &A) {
    double sum = 0;
    for (const auto& edge : *(G1->getEdgeList())) {
       uint node1 = edge[0], node2 = edge[1];
       if(G2->hasEdge(A[node1],A[node2]))
	   sum += getEdgeScore(G1->getEdgeWeight(node1,node2), G2->getEdgeWeight(A[node1],A[node2]));
    }
    return sum;
}

double EdgeGeoMean::getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    double val = computeIncChangeOp(peg, oldHole, newHole, A);
    assert(val == val); // will fail if NaN
    return val;
}

double EdgeGeoMean::computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A) {
    double diff = 0;
    for (uint nbr : *(G1->getAdjList(peg))) {
	if(G2->hasEdge(oldHole, A[nbr]))
	    diff -= getEdgeScore(G1->getEdgeWeight(peg, nbr), G2->getEdgeWeight(oldHole, A[nbr]));

        uint nbrHole = nbr == peg ? newHole : A[nbr];
	if(G2->hasEdge(newHole, nbrHole))
	    diff += getEdgeScore(G1->getEdgeWeight(peg, nbr), G2->getEdgeWeight(newHole, nbrHole));
    }
    return diff;
}

double EdgeGeoMean::getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    return computeIncSwapOp(peg1, peg2, hole1, hole2, A);
}

/* We swap the mapping of two nodes peg1 and peg2
 * We can first handle peg1, then do the same with peg2
 * Subtract old edge value with edge (peg1, hole1)
 * Add new edge value with edge (peg1, hole2) */
double EdgeGeoMean::computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A) {
    if (peg1 == peg2) return 0;
    // Handle peg1
    double diff = 0;
    double c = 0;
    for (uint nbr : *(G1->getAdjList(peg1))) {
        if(G2->hasEdge(hole1, A[nbr]))
	    diff -= getEdgeScore(G1->getEdgeWeight(peg1, nbr), G2->getEdgeWeight(hole1, A[nbr]));

        // Determine the new target hole for nbr
        uint nbrHole = 0;
        if (nbr == peg1) nbrHole = hole2;
        else if (nbr == peg2) nbrHole = hole1;
        else nbrHole = A[nbr];

        if(G2->hasEdge(hole2, nbrHole))
	    diff += getEdgeScore(G1->getEdgeWeight(peg1, nbr), G2->getEdgeWeight(hole2, nbrHole));
    }
    // Handle peg2
    for (uint nbr : *(G1->getAdjList(peg2))) {
        if (nbr == peg1) continue;
        if(G2->hasEdge(hole2, A[nbr]))
	    diff -= getEdgeScore(G1->getEdgeWeight(peg2, nbr), G2->getEdgeWeight(hole2, A[nbr]));

        uint nbrHole = (nbr == peg2 ? hole1 : A[nbr]);
        if(G2->hasEdge(hole1, nbrHole))
	    diff += getEdgeScore(G1->getEdgeWeight(peg2, nbr), G2->getEdgeWeight(hole1, nbrHole));
    }
    return diff;
}


