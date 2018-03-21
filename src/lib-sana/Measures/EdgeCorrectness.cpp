//
// Created by taylor on 3/21/18.
//

#include "Alignment.hpp"
#include "EdgeCorrectness.hpp"

EdgeCorrectness::EdgeCorrectness(Graph *G1, Graph *G2) : Measure(G1,G2,"ec") {}

EdgeCorrectness::~EdgeCorrectness() {}

double EdgeCorrectness::eval(const Alignment &A) {
    return static_cast<double>
    (A.numAlignedEdges(*G1,*G2)/G1->GetNumEdges());
}