#include <vector>
#include <iostream>
#include <sstream>
#include "SPINALWrapper.hpp"

using namespace std;

const string CONVERTER = "python converter_to_gml.py";

SPINALWrapper::SPINALWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "SPINAL", args) {
	wrappedDir = "wrappedAlgorithms/SPINAL";
}

void SPINALWrapper::loadDefaultParameters() {
	parameters = "";
}

string SPINALWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile  = name + ".gw";
	string gmlFile = name + ".gml";

	graph->saveInGWFormat(gwFile);

	exec("mv " + gwFile + " " + wrappedDir);

	exec("cd " + wrappedDir + "; chmod +x " + CONVERTER );
	exec("cd " + wrappedDir + "; " + CONVERTER + " " + gwFile);
	exec("mv " + wrappedDir + "/" + gmlFile + " " + gmlFile);

	return gmlFile;
}

string SPINALWrapper::generateAlignment() {
	return " ";
}

Alignment SPINALWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	//TODO replace
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    return Alignment(mapping);
}

void SPINALWrapper::deleteAuxFiles() {
//    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
