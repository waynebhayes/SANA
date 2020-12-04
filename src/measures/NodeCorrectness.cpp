#include "NodeCorrectness.hpp"
#include <vector>
#include <iostream>

using namespace std;

NodeCorrectness::NodeCorrectness(const Alignment& A): Measure(NULL, NULL, "nc"),
    trueA(A) {
}

NodeCorrectness::~NodeCorrectness() {
}

double NodeCorrectness::eval(const Alignment& A) {
    uint count = 0;
    for (uint i = 0; i < A.size(); i++) {
        if (A[i] == trueA[i]) count++;
    }
    return (double) count / A.size();
}

bool NodeCorrectness::fulfillsPrereqs(Graph* G1, Graph* G2) {
    return G1->sameNodeNames(*G2);
}

double NodeCorrectness::eval(const MultiAlignment& MA){return 0;} //dummy declare
