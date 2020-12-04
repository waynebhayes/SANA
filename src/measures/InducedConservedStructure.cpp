#include <vector>
#include "InducedConservedStructure.hpp"

InducedConservedStructure::InducedConservedStructure(Graph* G1, Graph* G2) : Measure(G1, G2, "ics") {
}

InducedConservedStructure::~InducedConservedStructure() {
}

double InducedConservedStructure::eval(const Alignment& A) {
    return (double) A.numAlignedEdges(*G1, *G2)/G2->numNodeInducedSubgraphEdges(A.getMapping());
}

double InducedConservedStructure::eval(const MultiAlignment& MA){return 0;} //dummy declare
