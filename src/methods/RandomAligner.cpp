#include <iostream>
#include "RandomAligner.hpp"

using namespace std;

RandomAligner::RandomAligner(Graph* G1, Graph* G2): Method(G1, G2, "random") {

}

Alignment RandomAligner::run() {
    return Alignment::random(G1->getNumNodes(), G2->getNumNodes());
}

void RandomAligner::describeParameters(ostream& stream) {

}

string RandomAligner::fileNameSuffix(const Alignment& A) {
    return "";
}
