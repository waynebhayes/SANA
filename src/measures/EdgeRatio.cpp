#include "EdgeRatio.hpp"
#include "../utils/utils.hpp"
#include <vector>
#include <array>

const Graph *EdgeRatio::G1, *EdgeRatio::G2;
double EdgeRatio::denominator;

EdgeRatio::EdgeRatio(const Graph* G1, const Graph* G2): Measure(G1, G2, "er") {
    assert(EdgeRatio::denominator==0);
    assert(EdgeRatio::G1==NULL);
    assert(EdgeRatio::G2==NULL);
    EdgeRatio::G1=G1;
    EdgeRatio::G2=G2;
    EdgeRatio::denominator=computeDenom();
}
EdgeRatio::~EdgeRatio() {}

double EdgeRatio::computeDenom() {
    return G1->getNumEdges();
}

double EdgeRatio::getSum(const Alignment &A) {
    return computeSum(A);
}

double EdgeRatio::eval(const Alignment& A) {
#ifndef WEIGHT
    return kErrorScore;
#else
    return computeSum(A);
#endif
}

double EdgeRatio::getRatio(double w1, double w2) {
    if (w1 == 0 and w2 == 0) return 0;
    double r = (abs(w1) < abs(w2) ? w1/w2 : w2/w1);
    assert(r >= -1 and r <= 1);
    // At this point, r is in [-1,1], but we want it in [0,1], so add 1 and divide by 2
    // r = (r+1)/2; assert(r >= 0 and r <= 1);
    return r;
}

double EdgeRatio::getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2){
    // The maximum possible score is attained during a correct self-alignment, in which case every edge has a ratio of 1.
    return computeAligEdgeScore(u1, v1, u2, v2);
}

double EdgeRatio::computeAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2) {
    return getRatio(G1->getEdgeWeight(u1, v1), G2->getEdgeWeight(u2, v2)) / denominator;
}

#define MALE_FLY_EDGES 4158055
#define MAX_A_ARRAY (8*MALE_FLY_EDGES)
static double a[MAX_A_ARRAY];

double EdgeRatio::computeSum(const Alignment &A) {
#ifndef WEIGHT
    return 0;
#else
    int ai=0, aSize=G1->getNumEdges();
    assert(aSize <= MAX_A_ARRAY);
    for (const auto& edge : *(G1->getEdgeList())) {
	uint node1 = edge[0], node2 = edge[1];
	a[ai++] = getAligEdgeScore(node1,node2,A[node1],A[node2]);
	// We don't need to include the reverse edge here in the directed graph case, because *if* a reverse edge of
	// (u,v) exists, it's actually *in* this edgeList.
	assert(ai<=aSize);
    }
    return AccurateSum(ai, a);
#endif
}

double EdgeRatio::getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A) {
    double val = computeIncChangeOp(peg, oldHole, newHole, A);
    assert(val == val); // check for NaN
    return val;
}

double EdgeRatio::computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A) {
    assert(A[peg] == oldHole);
    int ai=0, aSize=4*G1->getNumNodes(); // edges in both directions from everyone (including SELF!)
    assert(aSize <= MAX_A_ARRAY);
    auto k = G1->getAdjList(peg);
    for (uint nbr : *k) {
	if(nbr == peg) assert(A[nbr] == oldHole);
        a[ai++] = -getAligEdgeScore(peg,nbr ,oldHole,A[nbr]);
	// if the PEG has a self-loop, then any underlying self-loop is at newHole, otherwise
	// the underlying edge is between newHole and the true neighbor's aligned hole.
        uint nbrHole = (nbr == peg) ? newHole : A[nbr];
        a[ai++] = getAligEdgeScore(peg,nbr, newHole,nbrHole);
	assert(ai<=aSize);
    }
    if(G1->directed) {
        k = G1->getInjList(peg);
        for (uint nbr : *k) {
            if (nbr == peg)
                assert(A[nbr] == oldHole);
            a[ai++] = -getAligEdgeScore(nbr, peg, A[nbr], oldHole);
            uint nbrHole = (nbr == peg) ? newHole : A[nbr];
            a[ai++] = getAligEdgeScore(nbr, peg, nbrHole, newHole);
            assert(ai<=aSize);
        }
    }
    return AccurateSum(ai, a);
}

double EdgeRatio::getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A) {
    double val = computeIncSwapOp(peg1, peg2, hole1, hole2, A);
    assert(val == val); // check for NaN
    return val;
}

double EdgeRatio::computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A) {
    assert(A[peg1] == hole1 && A[peg2] == hole2);
    if (peg1 == peg2) return 0;
    int ai=0, aSize=8*G1->getNumNodes(); // two pegs, each with edges in both directions from potentially everyone else
    assert(aSize <= MAX_A_ARRAY);
    // Subtract (peg1->hole1), add (peg1->hole2)
    auto k = G1->getAdjList(peg1);
    for (uint nbr : *k) {
        if(nbr == peg1) assert(A[nbr] == hole1);
        a[ai++] = -getAligEdgeScore(peg1,nbr, hole1,A[nbr]);
        uint nbrHole; if(nbr == peg1) nbrHole=hole2; else if(nbr==peg2) nbrHole=hole1; else nbrHole=A[nbr];
        a[ai++] =  getAligEdgeScore(peg1,nbr, hole2,nbrHole);
        assert(ai<=aSize);
    }
    if(G1->directed) {
        k = G1->getInjList(peg1);
        for (uint nbr : *k) {
            if (nbr == peg1)
                assert(A[nbr] == hole1);
            a[ai++] = -getAligEdgeScore(nbr, peg1, A[nbr], hole1);
            uint nbrHole;
            if (nbr == peg1) nbrHole = hole2;
            else if (nbr == peg2) nbrHole = hole1;
            else nbrHole = A[nbr];
            a[ai++] = getAligEdgeScore(nbr, peg1, nbrHole, hole2);
            assert(ai<=aSize);
        }
    }
    // Subtract peg2-hole2, add peg2-hole1
    k = G1->getAdjList(peg2);
    for (uint nbr : *k) {
        if(nbr == peg2) assert(A[nbr] == hole2);
        a[ai++] = -getAligEdgeScore(peg2,nbr, hole2,A[nbr]);
        uint nbrHole; if(nbr == peg2) nbrHole=hole1; else if(nbr==peg1) nbrHole=hole2; else nbrHole=A[nbr];
        a[ai++] =  getAligEdgeScore(peg2,nbr, hole1,nbrHole);
        assert(ai<=aSize);
    }
    if(G1->directed) {
        k = G1->getInjList(peg2);
        for (uint nbr : *k) {
            if(nbr == peg2) assert(A[nbr] == hole2);
            a[ai++] = -getAligEdgeScore(nbr,peg2, A[nbr],hole2); uint nbrHole;
            if(nbr == peg2) nbrHole=hole1;
            else if(nbr==peg1) nbrHole=hole2;
            else nbrHole=A[nbr];
            a[ai++] =  getAligEdgeScore(nbr,peg2, nbrHole,hole1);
            assert(ai<=aSize);
        }
    }
    return AccurateSum(ai,a);
}
