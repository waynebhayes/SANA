#include <vector>
#include <iostream>
#include <sstream>
#include "GREATWrapper.hpp"

using namespace std;

const string CONVERTER = "bio-graph";
const string GHOSTBinary = "GHOST";

GREATWrapper::GREATWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GHOST", args) {
	wrappedDir = "wrappedAlgorithms/GHOST";
}

void GREATWrapper::loadDefaultParameters() {
	parameters = "";
}

string GREATWrapper::convertAndSaveGraph(Graph* graph, string name) {
	return name;
}

string GREATWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; chmod +x "+ GHOSTBinary);
	return " ";
}

Alignment GREATWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	//TODO replace
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    return Alignment(mapping);
}

void GREATWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
