#include "EdgeMin.hpp"
#include "../utils/utils.hpp"
#include <vector>
#include <array>

const Graph *EdgeMin::G1, *EdgeMin::G2;
double EdgeMin::denominator;

EdgeMin::EdgeMin(const Graph* G1, const Graph* G2): Measure(G1, G2, "emin") {
    assert(EdgeMin::denominator==0);
    assert(EdgeMin::G1==NULL);
    assert(EdgeMin::G2==NULL);
    EdgeMin::G1=G1;
    EdgeMin::G2=G2;
    EdgeMin::denominator=computeDenom();
    // Ensure we don't try to set them twice
}

EdgeMin::~EdgeMin() 
{}

double EdgeMin::computeDenom() {
    // NOTE: getTotalEdgeWeight doesn't work because, if the edges are signed, the result is close to zero.
    // double val = min(G1->getTotalEdgeWeight(), G2->getTotalEdgeWeight());
    double ew, sumG1=0, sumG2=0;

    // Sum edge weights of both graphs
    for (const auto& edge : *(G1->getEdgeList())) {
	ew = G1->getEdgeWeight(edge[0], edge[1]);
	sumG1 += ew;
    }
    for (const auto& edge : *(G2->getEdgeList())) {
	ew = G2->getEdgeWeight(edge[0], edge[1]);
	sumG2 += ew;
    }

    return std::min(sumG1, sumG2);
}

double EdgeMin::eval(const Alignment& A) {
    return computeSum(A);
}

double EdgeMin::getAligEdgeScore(const uint peg1, const uint peg2, const uint hole1, const uint hole2) {
    return computeAligEdgeScore(peg1, peg2, hole1, hole2);
}

double EdgeMin::computeAligEdgeScore(const uint peg1, const uint peg2, const uint hole1, const uint hole2){
    assert(0 <= peg1  && peg1  < G1->getNumNodes());
    assert(0 <= peg2  && peg2  < G1->getNumNodes());
    assert(0 <= hole1 && hole1 < G2->getNumNodes());
    assert(0 <= hole2 && hole2 < G2->getNumNodes());
    assert(EdgeMin::denominator != 0);
    const double smaller = getMin(G1->getEdgeWeight(peg1, peg2), G2->getEdgeWeight(hole1, hole2));
    assert(smaller == smaller); // Checks `smaller` isn't NaN

    return smaller / EdgeMin::denominator;
}

#define MALE_FLY_EDGES (4158055+1000) // 1000 is extra space just to be sure
#define MAX_A_ARRAY (8*MALE_FLY_EDGES)
static double a[MAX_A_ARRAY];

// Score the contribution of peg's aligned edges while in hole1, optionally avoiding "avoidPeg" if it's a neighbor.
// Thanks to Marcus Longo for this idea (2025-01-27)
double EdgeMin::scoreOnePegSlow(const uint peg, const uint avoidPeg, const uint hole, const Alignment& A) {
    auto pegAdjList = G1->getAdjList(peg);
    auto pegInjList = G2->getInjList(peg);
    int ai=0, aSize=pegAdjList->size() + pegInjList->size() + 3; // 3 subtractions below
    assert(aSize <= MAX_A_ARRAY);

    // Process edges emanating from peg
    for(const auto& nbr : *pegAdjList) {
	if(nbr!=avoidPeg) { // FIXME: can we avoid the branch by subtracting below the loop?
            a[ai++] = computeAligEdgeScore(peg, nbr, hole, A[nbr]);
        }
    }
    assert(ai<=aSize);

    // Process edges targeting peg EXCEPT for any self-loop, which was already counted above.
    for(const auto& nbr : *pegInjList) {
	if(nbr!=avoidPeg && nbr!=peg) { // FIXME: can we avoid the branch by subtracting below the loop?
            a[ai++] = computeAligEdgeScore(nbr, peg, A[nbr], hole); 
        }
    }
    assert(ai<=aSize);

    return AccurateSum(ai, a);
}

double EdgeMin::getSum(const Alignment &A) {
    return computeSum(A);
}

double EdgeMin::computeSum(const Alignment &A) {
#ifndef WEIGHT
    return 0;
#else
    int ai=0, aSize=G1->getNumEdges();
    assert(aSize <= MAX_A_ARRAY);
    for (const auto& edge : *(G1->getEdgeList())) {
	uint node1 = edge[0], node2 = edge[1];
	a[ai++] = computeAligEdgeScore(node1, node2, A[node1], A[node2]);
	// NOTE: We don't need to include the reverse edge here in the directed graph case, because *if* a reverse edge of
	// (u,v) exists, it's actually *in* this edgeList.
	assert(ai<=aSize);
    }

    return AccurateSum(ai, a);
#endif
}

double EdgeMin::getMin(const double w1, const double w2) {
    static bool warned;
    if(!warned && (w1<0 || w2<0)) {
	if(w1<0) cerr << "WARNING: EdgeMin really doesn't make sense with negative edge (eg., " << w1 << ")\n";
	if(w2<0) cerr << "WARNING: EdgeMin really doesn't make sense with negative edge (eg., " << w2 << ")\n";
	warned=true;
    }
    return std::min(w1, w2);
}

double EdgeMin::getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    double val = computeIncChangeOp(peg, oldHole, newHole, A);
    assert(val == val); // will fail if NaN
    return val;
}

double EdgeMin::computeIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A) {
    int ai=0, aSize=4*G1->getNumNodes(); // edges in both directions from everyone (including SELF!)
    assert(A[peg] == oldHole);
    assert(aSize <= MAX_A_ARRAY);

    auto pegAdjList = G1->getAdjList(peg);
    for(const auto& nbr : *pegAdjList) {
	if(nbr == peg) assert(A[nbr] == oldHole);
        a[ai++] = -computeAligEdgeScore(peg, nbr, oldHole, A[nbr]);
	// NOTE: if the PEG has a self-loop, then moving it to newHole means we need to check for underlying self-loop at newHole;
	//       otherwise the underlying edge is between newHole and the (non-self) neighbor's aligned hole.
        uint nbrHole = (nbr == peg) ? newHole : A[nbr];
        a[ai++] =  computeAligEdgeScore(peg,nbr, newHole,nbrHole);
	assert(ai<=aSize);
    }

    if (G1->directed) {
        auto pegInjList = G2->getInjList(peg);
        for (const auto& nbr : *pegInjList) {
            if(nbr == peg) assert(A[nbr] == oldHole);
            a[ai++] = -computeAligEdgeScore(nbr,peg, A[nbr],oldHole);
            uint nbrHole = (nbr == peg) ? newHole : A[nbr];
            a[ai++] =  computeAligEdgeScore(nbr,peg, nbrHole,newHole);
            assert(ai<=aSize);
        }
    } 

    double old = AccurateSum(ai, a);
#if DEBUG_EDGEMIN
// WARNING: actually, now that I've moved the incremental code here and am using copies of G1 and G2, it's scoreOnePeg
//          that is broken... but it's NOT when I leave it in SANA.cpp???
    double noAvoid = G1->getNumNodes();
    double before = scoreOnePegSlow(peg, noAvoid, oldHole, A);
    double after  = scoreOnePegSlow(peg, noAvoid, newHole, A);
    static int bad; 
    printf("[%d bad] old %g Marcus %g diff %g\n", bad, old, after-before, after-before-old); 
    assert(++bad<999);
    printf("O"); fflush(stdout);
    if(fabs(after-before-old)<1e-19) return old;
#endif
    return old;
}

double EdgeMin::getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    return computeIncSwapOp(peg1, peg2, hole1, hole2, A);
}

double EdgeMin::computeIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    int ai=0, aSize=8*G1->getNumNodes(); // two pegs, each with edges in both directions from potentially everyone else
    assert(peg1 != peg2);
    assert(A[peg1] == hole1 && A[peg2] == hole2);
    assert(aSize <= MAX_A_ARRAY);

    // Subtract (peg1->hole1), add (peg1->hole2)
    uint nbrHole;
    auto k = G1->getAdjList(peg1);
    for (const auto& nbr : *k) {
	if(nbr == peg1) assert(A[nbr] == hole1);
        a[ai++] = -computeAligEdgeScore(peg1,nbr, hole1,A[nbr]);
        if(nbr == peg1)    nbrHole=hole2; 
        else if(nbr==peg2) nbrHole=hole1; 
        else               nbrHole=A[nbr];
        a[ai++] =  computeAligEdgeScore(peg1,nbr, hole2,nbrHole);
	assert(ai<=aSize);
    }

    if(G1->directed) {
        k = G2->getInjList(peg1);
        for (const auto& nbr : *k) { // skip the self and peg2 outgoing
            if(nbr == peg1) assert(A[nbr] == hole1);
            if(nbr != peg1 && nbr!=peg2) {
                a[ai++] = -computeAligEdgeScore(nbr, peg1, A[nbr], hole1);
                nbrHole=A[nbr];
                a[ai++] =  computeAligEdgeScore(nbr, peg1, nbrHole, hole2);
            }
            assert(ai<=aSize);
        }
    }

   // Subtract peg2-hole2, add peg2-hole1
    k = G1->getAdjList(peg2);
   for (const auto& nbr : *k) {
	if(nbr == peg2) assert(A[nbr] == hole2);
        a[ai++] = -computeAligEdgeScore(peg2,nbr, hole2,A[nbr]);
        if(nbr == peg2)    nbrHole=hole1; 
        else if(nbr==peg1) nbrHole=hole2; 
        else               nbrHole=A[nbr];
        a[ai++] =  computeAligEdgeScore(peg2,nbr, hole1,nbrHole);
	assert(ai<=aSize);
    }

    if(G1->directed) {
        for (const auto& nbr : *(G1->getInjList(peg2))) {
            if(nbr == peg2) assert(A[nbr] == hole2);
            if(nbr != peg2 && nbr!=peg1) {
                a[ai++] = -computeAligEdgeScore(nbr,peg2,A[nbr],hole2);
                nbrHole=A[nbr];
                a[ai++] =  computeAligEdgeScore(nbr,peg2,nbrHole,hole1);
            }
            assert(ai<=aSize);
        }
    }
    double oldFast = AccurateSum(ai,a);

#if DEBUG_EDGEMIN
    double easySlow = IncSwapOp2(peg1, peg2, hole1, hole2, A);
    if(std::fabs(oldFast - easySlow) > 4e-16) printf("X");fflush(stdout)

    //_predictedScore1 = currentScore + oldFast; _predictedScore2 = currentScore + easySlow;
    if(std::fabs(easySlow - oldFast) > 1e-8)  printf("oldIncSwap = %g, new = %g, diff = %g \n", oldFast, easySlow, easySlow-oldFast);
    return easySlow;
#else
    return oldFast;
#endif
}

double EdgeMin::getIncSwapOp2(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A) {
    return computeIncSwapOp2(peg1, peg2, hole1, hole2, A);
}

double EdgeMin::computeIncSwapOp2(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &AA) {
    Alignment A = AA;
    assert(A[peg1] == hole1 && A[peg2] == hole2);
    assert(peg1 != peg2); // return 0;
    double noAvoid = G1->getNumNodes();
    double old = scoreOnePegSlow(peg1, noAvoid, hole1, A); // score outward and inward A-aligned edges of peg1
    old +=       scoreOnePegSlow(peg2, peg1, hole2, A); // score out&in as above for peg2 EXCEPT if going to peg1

    // NOTE: we must PHYSICALLY swap peg1+peg2 in A, in order to correctly score the new position
    A.swap(peg1, peg2);
    double New = scoreOnePegSlow(peg2,noAvoid, hole1, A); // score outward and inward aligned edges of peg1
    New +=       scoreOnePegSlow(peg1,peg2,    hole2, A); // score out&in as above for peg2 EXCEPT if going to peg1
    A.swap(peg1, peg2);
    return New - old;
}
