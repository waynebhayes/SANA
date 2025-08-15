#include "Misc.hpp"
#include "EdgeCorrectness.hpp"
#include <string>
#include <vector>

const Graph* EdgeCorrectness::G1 = nullptr;
const Graph* EdgeCorrectness::G2 = nullptr;
double EdgeCorrectness::denominator = 1;
EdgeCorrectness::EdgeCorrectness(const Graph* G1, const Graph* G2, int graphNum): Measure(G1, G2, "ec") {
    EdgeCorrectness::G1=G1;
    EdgeCorrectness::G2=G2;
    switch (graphNum) {
        case 1: denominator = G1->getNumEdges(); break;
        case 2: denominator = G2->getNumEdges(); break;
        default: Fatal("unknown graphNum %d in EdgeCorrectness::eval", graphNum);
    }
}

EdgeCorrectness::~EdgeCorrectness() {
}

double EdgeCorrectness::eval(const Alignment& A) {
    return A.computeNumAlignedEdges(*G1, *G2) / denominator;
}

double EdgeCorrectness::getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A) {
    int res = 0;
    if (G1->hasSelfLoop(peg)) {
        if (G2->hasSelfLoop(oldHole)) res-=G2->getEdgeWeight(oldHole, oldHole);
        if (G2->hasSelfLoop(newHole)) res+=G2->getEdgeWeight(newHole, newHole);
    }
    for (const auto& nbr : *G1->getAdjList(peg)) if (nbr != peg) {
	res -= G2->getEdgeWeight(oldHole, A[nbr]);
	res += G2->getEdgeWeight(newHole, A[nbr]);
    }
    if(G1->directed) for (const auto& nbr : *G1->getInjList(peg)) if (nbr != peg) {
	res -= G2->getEdgeWeight(A[nbr],oldHole);
	res += G2->getEdgeWeight(A[nbr],newHole);
    }
    return res / denominator;
}

double EdgeCorrectness::getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A) {
    int res = 0;
    if (G1->hasSelfLoop(peg1)) {
        if (G2->hasSelfLoop(hole1)) res-=G2->getEdgeWeight(hole1, hole1);
        if (G2->hasSelfLoop(hole2)) res+=G2->getEdgeWeight(hole2, hole2);
    }
    for (const auto& nbr : *G1->getAdjList(peg1)) if (nbr != peg1) {
	res -= G2->getEdgeWeight(hole1, A[nbr]);
	res += G2->getEdgeWeight(hole2, A[nbr]);
    }
    if(G1->directed) for (const auto& nbr : *G1->getInjList(peg1)) if (nbr != peg1) {
	res -= G2->getEdgeWeight(A[nbr],hole1);
	res += G2->getEdgeWeight(A[nbr],hole2);
    }

    if (G1->hasSelfLoop(peg2)) {
        if (G2->hasSelfLoop(hole2)) res-=G2->getEdgeWeight(hole2, hole2);
        if (G2->hasSelfLoop(hole1)) res+=G2->getEdgeWeight(hole1, hole1);
    }
    for (const auto& nbr : *G1->getAdjList(peg2)) if (nbr != peg2) {
	res -= G2->getEdgeWeight(hole2, A[nbr]);
	res += G2->getEdgeWeight(hole1, A[nbr]);
    }
    if(G1->directed) for (const auto& nbr : *G1->getInjList(peg2)) if (nbr != peg2) {
	res -= G2->getEdgeWeight(A[nbr],hole2);
	res += G2->getEdgeWeight(A[nbr],hole1);
    }

    // Currently commented out because I fail to comprehend this. Multi_pairwise will be refactored
    // anyway. -Marcus

    //address the case where we are swapping between adjacent nodes with adjacent images
// #if defined(MULTI_PAIRWISE) || defined(MULTI_MPI)
//     //why set the least-significant bit to 0?
//     //this kind of bit manipulation needs a comment clarification -Nil
//     res += (-1 << 1) & (G1->getEdgeWeight(peg1, peg2) +
//                         G2->getEdgeWeight(hole1, hole2));
// #else
     if                 (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2)) res += 2;
     if(G1->directed) if(G1->hasEdge(peg2, peg1) and G2->hasEdge(hole2, hole1)) res += 2;
// #endif
    return res / denominator;
}
