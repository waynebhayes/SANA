#include "JaccardSimilarityScore.hpp"
#include <vector>

namespace {
double jsNodeContribution(const Graph* G1, const Graph* G2, uint node, uint hole, uint alignedEdges) {
    uint denominator = G1->getNumNbrs(node) + G2->getNumNbrs(hole) - alignedEdges;
    if (denominator == 0) return 0;
    return alignedEdges / (double) denominator;
}

void addAffectedNode(vector<uint>& affectedNodes, vector<bool>& seen, uint node) {
    if (not seen[node]) {
        seen[node] = true;
        affectedNodes.push_back(node);
    }
}

vector<uint> affectedNodesChange(const Graph* G1, uint peg) {
    vector<uint> affectedNodes;
    vector<bool> seen(G1->getNumNodes(), false);
    addAffectedNode(affectedNodes, seen, peg);
    if (G1->isDirected()) {
        for (uint nbr : *(G1->getInjList(peg))) addAffectedNode(affectedNodes, seen, nbr);
    } else {
        for (uint nbr : *(G1->getAdjList(peg))) addAffectedNode(affectedNodes, seen, nbr);
    }
    return affectedNodes;
}

vector<uint> affectedNodesSwap(const Graph* G1, uint peg1, uint peg2) {
    vector<uint> affectedNodes;
    vector<bool> seen(G1->getNumNodes(), false);
    addAffectedNode(affectedNodes, seen, peg1);
    addAffectedNode(affectedNodes, seen, peg2);
    if (G1->isDirected()) {
        for (uint nbr : *(G1->getInjList(peg1))) addAffectedNode(affectedNodes, seen, nbr);
        for (uint nbr : *(G1->getInjList(peg2))) addAffectedNode(affectedNodes, seen, nbr);
    } else {
        for (uint nbr : *(G1->getAdjList(peg1))) addAffectedNode(affectedNodes, seen, nbr);
        for (uint nbr : *(G1->getAdjList(peg2))) addAffectedNode(affectedNodes, seen, nbr);
    }
    return affectedNodes;
}

template<typename HoleFn>
uint alignedEdgesAtNode(const Graph* G1, const Graph* G2, uint node, const HoleFn& holeOf) {
    uint alignedEdges = 0;
    for (uint nbr : *(G1->getAdjList(node))) alignedEdges += G2->getEdgeWeight(holeOf(node), holeOf(nbr));
    return alignedEdges;
}
}

JaccardSimilarityScore::JaccardSimilarityScore(const Graph* G1, const Graph* G2): Measure(G1, G2, "js") {}

JaccardSimilarityScore::~JaccardSimilarityScore() {}

double JaccardSimilarityScore::eval(const Alignment& A) {
    double jsSum = 0;
    vector<uint> alignedByNode = JaccardSimilarityScore::getAlignedByNode(G1, G2, A);

    for (uint i = 0; i< G1->getNumNodes(); i++){
        jsSum += jsNodeContribution(G1, G2, i, A[i], alignedByNode[i]);
    }
    return jsSum;
}

vector<uint> JaccardSimilarityScore::getAlignedByNode(const Graph *G1, const Graph *G2, const Alignment& A){
    vector<uint> alignedByNode;
    for (uint i = 0; i < G1->getNumNodes(); i++){
        // dont need to check if i is aligned because g1 is smaller and always will be alinged
        uint iAlignedTo = A[i]; //find the node i is mapped to
        uint iAlignedEdges = 0;
        for (uint nbr : *(G1->getAdjList(i))) {
            uint neighbourAlignedTo = A[nbr];
            iAlignedEdges += G2->getEdgeWeight(iAlignedTo, neighbourAlignedTo);
        }
        alignedByNode.push_back(iAlignedEdges);
    }
    return alignedByNode;
}

double JaccardSimilarityScore::getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment& A) {
    (void) oldHole;
    double change = 0;
    vector<uint> alignedByNode = getAlignedByNode(G1, G2, A);
    vector<uint> affectedNodes = affectedNodesChange(G1, peg);
    auto holeOf = [&](uint node) -> uint {
        return node == peg ? newHole : A[node];
    };
    for (uint node : affectedNodes) {
        uint newAlignedEdges = alignedEdgesAtNode(G1, G2, node, holeOf);
        change += jsNodeContribution(G1, G2, node, holeOf(node), newAlignedEdges)
            - jsNodeContribution(G1, G2, node, A[node], alignedByNode[node]);
    }
    return change;
}

double JaccardSimilarityScore::getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment& A) {
    double change = 0;
    vector<uint> alignedByNode = getAlignedByNode(G1, G2, A);
    vector<uint> affectedNodes = affectedNodesSwap(G1, peg1, peg2);
    auto holeOf = [&](uint node) -> uint {
        if (node == peg1) return hole2;
        if (node == peg2) return hole1;
        return A[node];
    };
    for (uint node : affectedNodes) {
        uint newAlignedEdges = alignedEdgesAtNode(G1, G2, node, holeOf);
        change += jsNodeContribution(G1, G2, node, holeOf(node), newAlignedEdges)
            - jsNodeContribution(G1, G2, node, A[node], alignedByNode[node]);
    }
    return change;
}
