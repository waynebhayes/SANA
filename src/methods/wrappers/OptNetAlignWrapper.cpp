#include <vector>
#include <iostream>
#include <sstream>
#include "OptNetAlignWrapper.hpp"

using namespace std;

const string CONVERTER = "python converter.py";
const string PROGRAM   = "./optnetalign";

OptNetAlignWrapper::OptNetAlignWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "OPTNETALIGN", args) {
	wrappedDir = "wrappedAlgorithms/OptNetAlign";
}

void OptNetAlignWrapper::loadDefaultParameters() {
	parameters = "--ec --s3 --generations 2000 --outprefix --result";
}

string OptNetAlignWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string txtFile = name + ".txt";
	graph->writeGraphEdgeListFormat(txtFile);
	return txtFile;
}

string OptNetAlignWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);
	exec("cd " + wrappedDir + "; " + PROGRAM + " --net1 " +
			g1File + " --net2 " + g2File + " " + parameters);

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
