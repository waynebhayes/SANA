#include <vector>
#include <iostream>
#include <sstream>
#include "PISwapWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PISwapBinary = "PISWAP";

PISwapWrapper::PISwapWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "PISwap", args) {
	wrappedDir = "wrappedAlgorithms/PISWAP";
}

void PISwapWrapper::loadDefaultParameters() {
	parameters = "";
}

string PISwapWrapper::convertAndSaveGraph(Graph* graph, string name) {
	return name;
}

string PISwapWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; chmod +x "+ PISwapBinary);
	return " ";
}

Alignment PISwapWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	//TODO replace
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    return Alignment(mapping);
}

void PISwapWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
