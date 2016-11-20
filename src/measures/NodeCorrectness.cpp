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
	//std::cout << count << "/" << trueA[trueA.size()-1] << std::endl;
    return (double) count / trueA[trueA.size()-1];
}

vector<ushort> NodeCorrectness::convertAlign(const Graph& G1, const Graph& G2, const vector<string>& E){
    map<string,ushort> mapG1 = G1.getNodeNameToIndexMap();
    map<string,ushort> mapG2 = G2.getNodeNameToIndexMap();
    vector<ushort> alignment(G1.getNumNodes(), G2.getNumNodes());
    for(unsigned int i = 0; i < E.size()/2; ++i)
    {
        string n1 = E[2*i];
	string n2 = E[2*i+1];
	alignment[mapG1[n1]] = mapG2[n2];
    }
    alignment.push_back(E.size()/2);
    return alignment;
}

vector<ushort> NodeCorrectness::getMappingforNC() const{
	return trueA.getMapping();
}

bool NodeCorrectness::fulfillsPrereqs(Graph* G1, Graph* G2) {
    return G1->sameNodeNames(*G2);
}
