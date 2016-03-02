#include <vector>
#include <iostream>
#include <sstream>
#include "GHOSTWrapper.hpp"

using namespace std;

const string CONVERTER = "bio-graph";
const string GHOSTBinary = "GHOST";

GHOSTWrapper::GHOSTWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GHOST", args) {
	wrappedDir = "wrappedAlgorithms/GHOST";
}

void GHOSTWrapper::loadDefaultParameters() {
	parameters = "";
}

string GHOSTWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile   = name + ".gw";
	string gexfFile = name + ".gexf";

	graph->saveInGWFormat(gwFile);

	exec("mv " + gwFile + " " + wrappedDir);
	exec("cd " + wrappedDir + "; chmod +x " + CONVERTER );
	exec("cd " + wrappedDir + "; " + CONVERTER + " --convert " + gwFile + " --output " + gexfFile + ".gexf");
	exec("mv " + wrappedDir + "/" + gexfFile + " " + gexfFile);

	return gexfFile;
}

string GHOSTWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; chmod +x "+ GHOSTBinary);
	return " ";
}

Alignment GHOSTWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	//TODO replace
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    return Alignment(mapping);
}

void GHOSTWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
