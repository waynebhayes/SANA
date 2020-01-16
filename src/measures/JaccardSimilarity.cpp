#include <vector>
#include "JaccardSimiliarity.hpp"

JaccardSimiliarity::JaccardSimiliarity(Graph* G1, Graph* G2) : Measure(G1, G2, "js") {
}

JaccardSimiliarity::~JaccardSimiliarity() {
}

double JaccardSimiliarity::eval(const Alignment& A) {
    
}
