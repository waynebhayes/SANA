#include "SymmetricEdgeCoverage.hpp"
#include <vector>
#include <iostream>
SymmetricEdgeCoverage::SymmetricEdgeCoverage(Graph* G1, Graph* G2) : Measure(G1, G2, "sec") {
}

SymmetricEdgeCoverage::~SymmetricEdgeCoverage() {
}

double SymmetricEdgeCoverage::eval(const Alignment& A) {
    double result = (double) A.numAlignedEdges(*G1, *G2)/G1->getNumEdges();
	//std::cout<<"check at SymmetricEdgeCoverage before reversing graphs order"<<std::endl;
    //result += (double) A.numAlignedEdges(*G2, *G1)/G2->getNumEdges();
	result += (double) A.numAlignedEdges(*G1, *G2)/G2->getNumEdges();
    result = result/2;

    /*std::cout<<"\n\n-sec print start";
    std::cout<<"\nA.numAlignedEdges(*G1, *G2) = "<<A.numAlignedEdges(*G1, *G2)<<"  G1->getNumEdges() = "<<G1->getNumEdges();
    std::cout<<"\nA.numAlignedEdges(*G2, *G1) = "<<A.numAlignedEdges(*G2, *G1);
    std::cout<<"  G2->getNumEdges() = "<<G2->getNumEdges();
    std::cout<<"\nresult = "<<result<<"\n\n";*/

    return result;
}

double SymmetricEdgeCoverage::eval(const MultiAlignment& MA){return 0;} //dummy declare
