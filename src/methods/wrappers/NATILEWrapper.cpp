#include <vector>
#include <iostream>
#include <sstream>
#include "NATILEWrapper.hpp"

using namespace std;

const string CONVERTER = "";

NATILEWrapper::NATILEWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GHOST", args) {
	wrappedDir = "wrappedAlgorithms/GHOST";
}

void NATILEWrapper::loadDefaultParameters() {
	parameters = "";
}

string NATILEWrapper::convertAndSaveGraph(Graph* graph, string name) {
	return name;
}

string NATILEWrapper::generateAlignment() {
	return " ";
}

Alignment NATILEWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	//TODO replace
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    return Alignment(mapping);
}

void NATILEWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
