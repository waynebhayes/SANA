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
    bool g2Used[G2.getNumNodes()];
    for(unsigned int i = 0; i < G2.getNumNodes(); ++i)
	g2Used[i] = false;
    for(unsigned int i = 0; i < E.size()/2; ++i)
    {
        string n1 = E[2*i];
	string n2 = E[2*i+1];
	unsigned int g1pos,g2pos;
	try{
	    g1pos = mapG1.at(n1);
	    g2pos = mapG2.at(n2);
	}catch(...){
		throw std::runtime_error("Truealignment contains a node which does not exist");
	}
	if(alignment[g1pos] != G2.getNumNodes() || g2Used[g2pos])
		throw std::runtime_error("Truealignment is not One to One");
	else
		g2Used[g2pos] = true;

	alignment[mapG1.at(n1)] = mapG2.at(n2);
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
