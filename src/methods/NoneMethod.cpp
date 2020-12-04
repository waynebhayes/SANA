#include <string>
#include <vector>
#include "NoneMethod.hpp"
using namespace std;

NoneMethod::NoneMethod(Graph* G1, Graph* G2, string startAName):
    Method(G1, G2, "None"), A(Alignment::empty())
{
    if (startAName == "") {
        uint n1 = G1->getNumNodes();
        uint n2 = G2->getNumNodes();
        A = Alignment(Alignment::random(n1, n2));
    }
    else {
        A = Alignment(Alignment::loadMapping(startAName));
    }

    A = Alignment::loadEdgeList(G1, G2, startAName);
}

NoneMethod::~NoneMethod() {
}

Alignment NoneMethod::run() {
    return A;
}

void NoneMethod::describeParameters(ostream& stream) {

}

string NoneMethod::fileNameSuffix(const Alignment& A) {
    return "none";
}
