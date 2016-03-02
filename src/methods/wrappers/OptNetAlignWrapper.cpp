#include <vector>
#include <iostream>
#include <sstream>
#include "OptNetAlignWrapper.hpp"

using namespace std;

const string CONVERTER = "python converter.py";

OptNetAlignWrapper::OptNetAlignWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GHOST", args) {
	wrappedDir = "wrappedAlgorithms/OptNetAlign";
}

void OptNetAlignWrapper::loadDefaultParameters() {
	parameters = "";
}

string OptNetAlignWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile  = name + ".gw";
	string txtFile = name + ".txt";

	graph->saveInGWFormat(gwFile);
	exec("mv " + gwFile + " " + wrappedDir);

	exec("cd " + wrappedDir + "; chmod +x " + CONVERTER );

	exec("cd " + wrappedDir + "; " + CONVERTER + " " + gwFile);
	exec("mv " + wrappedDir + "/" + txtFile + " " + txtFile);

	return txtFile;
}

string OptNetAlignWrapper::generateAlignment() {
	return " ";
}

Alignment OptNetAlignWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	//TODO replace
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    return Alignment(mapping);
}

void OptNetAlignWrapper::deleteAuxFiles() {
    //exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
